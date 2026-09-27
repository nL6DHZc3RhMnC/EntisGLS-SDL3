#include "legacy_compat/gls.h"
#include "legacy_compiler.h"
#include "platform/log.h"
#include <cstring>
#include <memory>

ESLError LegacyCalculateString(ECSString &string,ECSContext &context,ECSObjArray<ECSObject> &arguments) {
    auto error=context.VerifyArgumentCount(arguments,1,2);if(error)return error;
    if(!context.m_pcsxi)return ESLErrorMsg("String.Calculate requires an execution image");
    auto *errorMessage=ESLTypeCast<ECSString>(context.GetArgumentObjectAs(arguments,1,L"String"));
    ECSCompiler compiler;ECSSourceStream source;source=string.m_varStr;
    compiler.AttachExternalMacroVariable(&context.m_pcsxi->m_csgGlobal);
    error=compiler.InitConstExprContext(context.m_pcsxi);if(error)return error;
    struct Release {
        ECSCompiler &compiler;
        ~Release(){compiler.ReleaseConstExprContext();}
    } release{compiler};
    ECSObject *value=nullptr;
    error=compiler.CalculateExpression(value,source,0,nullptr,false);
    if(error) {
        if(errorMessage)errorMessage->m_varStr=GetESLErrorMsg(error);
        // The original evaluator owns cleanup on error; it may leave pValue
        // referring to an already freed operand. Never delete it a second time.
        value=new ECSReference;
    } else if(errorMessage)errorMessage->m_varStr=L"";
    return context.PushObject(value);
}

namespace {
class ExpressionImage final:public ECSExecutionImage {
public:
    ExpressionImage() {
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
bool Check(bool ok,const char *stage) {
    if(!ok)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy compiler probe FAIL: %s",stage);
    return ok;
}
std::unique_ptr<ECSObject> Evaluate(ECSContext &context,const wchar_t *expression,ECSString &error) {
    ECSString string(expression);ECSObjArray<ECSObject> args;
    args.Add(new ECSReference(&string));args.Add(new ECSReference(&error));
    if(LegacyCalculateString(string,context,args))return nullptr;
    return std::unique_ptr<ECSObject>(context.PopObject());
}
}
bool CheckLegacyCompiler(ECSEnvironment &environment) {
    ExpressionImage image;image.AttachCSEnvironment(&environment);
    ECSContext context;const auto *previous=ECotophaScript::GetPrimaryContext();
    struct Release {
        ECSContext &context;const ECSContext *previous;
        ~Release(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(const_cast<ECSContext*>(previous));}
    } release{context,previous};
    if(!Check(!context.InitializeContext(&image),"initialize expression image"))return false;
    auto *counter=new ECSInteger(9);image.m_csgGlobal.AddVariable(L"probeCounter",counter);
    auto *flags=new ECSHash;flags->m_varArray.SetAs(L"enabled",new ECSInteger(-1));
    image.m_csgGlobal.AddVariable(L"probeFlags",flags);
    ECSString error;
    auto value=Evaluate(context,L"1 + 2 * 3",error);
    INT64 integer=0;
    if(!Check(value&&!value->OperateInteger(integer)&&integer==7&&error.m_varStr.IsEmpty(),"arithmetic precedence"))return false;
    value=Evaluate(context,L"4294967296 + probeCounter",error);
    if(!Check(value&&!value->OperateInteger(integer)&&integer==4294967305LL,"64-bit expression and live global lookup"))return false;
    value=Evaluate(context,L"probeFlags.enabled && (probeCounter == 9)",error);
    int boolean=0;
    if(!Check(value&&!value->OperateBoolean(boolean)&&boolean&&error.m_varStr.IsEmpty(),"live Hash member and boolean condition"))return false;
    value=Evaluate(context,L"\"日本語\" + \"abc\"",error);
    auto *string=ESLTypeCast<ECSString>(value.get());
    if(!Check(string&&string->m_varStr==L"日本語abc","native Unicode string expression"))return false;
    value=Evaluate(context,L"probeCounter += 4",error);
    if(!Check(value&&counter->GetValue()==13&&error.m_varStr.IsEmpty(),"dynamic assignment changes actual global"))return false;
    value=Evaluate(context,L"unknown_probe_symbol + 1",error);
    auto *reference=ESLTypeCast<ECSReference>(value.get());
    if(!Check(reference&&!reference->m_pRef&&!error.m_varStr.IsEmpty(),"invalid expression returns null and diagnostic"))return false;
    value=Evaluate(context,L"1 / 0",error);reference=ESLTypeCast<ECSReference>(value.get());
    if(!Check(reference&&!reference->m_pRef&&!error.m_varStr.IsEmpty(),"division by zero returns real evaluator error"))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy compiler probe PASS: real Calculate parser, precedence, int64/global, member condition, Unicode, assignment, errors");
    return true;
}
