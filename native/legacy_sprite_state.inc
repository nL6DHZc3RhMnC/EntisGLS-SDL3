// The GLS3 Sprite record is split around its image references: visual parameters
// and animation curves are encoded by the drawing adapter using fixed LE scalars.
ESLError ECSSprite::Save(ESLFileObject& file,ECSContext& context) {
    GraphicsLock lock;
    if(legacyRestorePending_)return ESLErrorMsg("Sprite cannot save before restoration is committed");
    if(!legacyPage_.IsEmpty()&&!legacyReferences_[3].m_pRef)
        return ESLErrorMsg("Sprite skin page has no script ResourceManager provenance");
    auto* buffer=sprite_->GetFrameBuffer();
    if(buffer&&buffer->HasZBuffer())return ESLErrorMsg("Sprite Z-buffer serialization is not implemented");
    uint32_t format=UINT32_MAX,width=0,height=0;
    if(buffer) {
        SakuraGL::SGLImageInfo info;
        if(buffer->GetImage()->GetImageInfo(info))return eslErrGeneral;
        format=info.format?info.format:0x04000001;width=info.width;height=info.height;
    }
    IndexAllMember();
    LegacySave::Writer out{file};out.U32(1);out.Reference(legacyReferences_[2],context);
    out.U32(format);out.U32(width);out.U32(height);out.U32(0);
    out.String(legacyPage_);out.Reference(legacyReferences_[3],context);
    if(out.error)return out.error;
    if(const auto error=ECSResource::Save(file,context))return error;
    out.Object(legacyItemState_,context);out.Reference(legacyReferences_[4],context);out.U32(0);
    if(out.error)return out.error;
    if(const auto error=SaveLegacySpriteCallbacks(file,*this,context))return error;
    if(const auto error=SaveLegacySpriteVisual(file,*this))return error;
    out.I32(legacyImageFrame_);out.Rect(legacyImageView_);
    out.Reference(legacyReferences_[0],context);out.Reference(legacyReferences_[1],context);
    if(out.error)return out.error;
    return SaveLegacySpriteAnimation(file,*this);
}

ESLError ECSSprite::Load(ESLFileObject& file,ECSContext& context) {
    GraphicsLock lock;
    Release();ResetLegacySpriteCallbacks(*this);
    for(auto& ref:legacyReferences_)ref.SetReference(nullptr,&context);
    struct Restoring {bool& flag;Restoring(bool& f):flag(f){flag=true;}~Restoring(){flag=false;}} restoring(legacyRestoring_);
    LegacySave::Reader in{file};
    if(in.U32()!=1)return ESLErrorMsg("Saved Sprite uses an unsupported implicit main parent");
    in.Reference(legacyReferences_[2],context);
    legacyImageFormat_=in.U32();legacyImageWidth_=in.U32();legacyImageHeight_=in.U32();
    if(in.U32()!=0)return ESLErrorMsg("Saved Sprite Z-buffer is not supported");
    if(legacyImageWidth_>16384||legacyImageHeight_>16384||
       (legacyImageFormat_!=UINT32_MAX&&(!legacyImageWidth_||!legacyImageHeight_)))return eslErrInvalidParam;
    legacyPage_=in.String();in.Reference(legacyReferences_[3],context);
    if(in.error)return in.error;
    if(const auto error=ECSResource::Load(file,context))return error;
    in.Object(legacyItemState_,context);in.Reference(legacyReferences_[4],context);in.Reserved();
    if(in.error)return in.error;
    if(const auto error=LoadLegacySpriteCallbacks(file,*this,context))return error;
    if(const auto error=LoadLegacySpriteVisual(file,*this))return error;
    legacyImageFrame_=in.I32();legacyImageView_=in.Rect();
    in.Reference(legacyReferences_[0],context);in.Reference(legacyReferences_[1],context);
    if(in.error)return in.error;
    if(const auto error=LoadLegacySpriteAnimation(file,*this))return error;
    if(legacyImageFormat_!=UINT32_MAX) {
        const auto error=sprite_->CreateBuffer(legacyImageWidth_,legacyImageHeight_,legacyImageFormat_,
            (legacyImageFormat_&255)==SakuraGL::formatImageGray?8:32);
        if(error)return ESLError(error);
    }
    legacyRestorePending_=true;
    return eslErrSuccess;
}

ESLError ECSSprite::RestoreLegacyItems() {
    auto* formed=ESLTypeCast<SakuraGL::SGLSpriteFormed>(sprite_.get());
    auto missing=[&](const wchar_t* id,const wchar_t* property,const char* reason) {
        auto* manager=ESLTypeCast<ECSResourceManager>(legacyReferences_[3].m_pRef);
        EWideString path=L"<unknown>";
        if(auto* context=ECotophaScript::GetPrimaryContext())if(manager) {
            EMemoryFile source;
            if(!source.Create(128)&&!manager->Save(source,*context)) {
                source.SeekLarge(0,ESLFileObject::FromBegin);LegacySave::Reader in{source};path=in.String();
                if(in.error)path=L"<unreadable>";
            }
        }
        study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady",
            "Legacy Sprite restore item failure: object=%p type=%ls nativeID=%ls page=%ls skin=%ls manager=%p provider=%p item=%ls property=%ls children=%zu reason=%s",
            this,GetTypeName(),static_cast<const wchar_t*>(sprite_->GetID()),legacyPage_.CharPtr()?legacyPage_.CharPtr():L"",
            path.CharPtr()?path.CharPtr():L"",manager,formSkin_.get(),id?id:L"",property?property:L"",sprite_->GetChildCount(),reason);
        for(size_t n=0;n<std::min<size_t>(sprite_->GetChildCount(),12);++n)if(auto* child=sprite_->GetChildAt(n))
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy Sprite restore child[%zu]: id=%ls scriptOwner=%p native=%p",
                n,static_cast<const wchar_t*>(child->GetID()),owners.count(child)?owners.at(child):nullptr,child);
        return ESLErrorMsg(reason);
    };
    for(int i=0;i<legacyItemState_.m_varArray.GetSize();++i) {
        auto* entry=legacyItemState_.m_varArray.GetAt(i);if(!entry)continue;
        auto* state=ESLTypeCast<ECSHash>(entry->GetObject());if(!state)return eslErrInvalidParam;
        const wchar_t* id=entry->Tag();auto* item=Item(id);
        for(const wchar_t* key:{L"text",L"font",L"image"}) {
            auto* value=ESLTypeCast<ECSString>(state->m_varArray.GetAs(key));if(!value)continue;
            if(!formed)return ESLErrorMsg("Saved Sprite form property needs a native formed Sprite");
            if(!std::wcscmp(key,L"text"))formed->SetSpriteText(id,value->m_varStr);
            else if(!std::wcscmp(key,L"font"))formed->SetSpriteTextFont(id,value->m_varStr);
            else formed->SetSpriteImage(id,value->m_varStr);
        }
        if(auto* command=ESLTypeCast<ECSString>(state->m_varArray.GetAs(L"command"))) {
            if(!item||item->InvokeCommands(command->m_varStr))return missing(id,L"command","Saved Sprite command cannot be restored on its skin item");
        }
        for(const wchar_t* key:{L"transparency",L"enabled",L"priority",L"checked",L"vscrollrange",L"vscrollpos",L"hscrollrange",L"hscrollpos"}) {
            auto* value=ESLTypeCast<ECSInteger>(state->m_varArray.GetAs(key));if(!value)continue;
            const int number=value->GetInt();
            if(!item&&!std::wcscmp(key,L"checked")&&formed) {
                // Older Android builds incorrectly recorded no-op CheckButton
                // calls (the stock message page has no ID_SKIP/ID_SKIP2).
                // Replay the real optional-button API, which also does nothing
                // for an absent button, then discard only that stale property.
                formed->CheckSpriteButton(id,number!=0);
                state->m_varArray.RemoveAs(L"checked");
                study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
                    "Legacy Sprite restore optional CheckButton no-op: page=%ls item=%ls",
                    legacyPage_.CharPtr()?legacyPage_.CharPtr():L"",id?id:L"");
                continue;
            }
            if(!item)return missing(id,key,"Saved Sprite item cannot be found in the reloaded skin page");
            if(!std::wcscmp(key,L"transparency"))item->SetTransparency(number);
            else if(!std::wcscmp(key,L"enabled"))item->SetEnable(number!=0);
            else if(!std::wcscmp(key,L"priority"))item->ChangePriority(number);
            else if(!std::wcscmp(key,L"checked"))item->CheckButton(number!=0);
            else if(!std::wcscmp(key,L"vscrollrange"))item->SetScrollRange(number,SakuraGL::SGLSprite::scrollVert);
            else if(!std::wcscmp(key,L"vscrollpos"))item->SetScrollPos(number,SakuraGL::SGLSprite::scrollVert);
            else if(!std::wcscmp(key,L"hscrollrange"))item->SetScrollRange(number,SakuraGL::SGLSprite::scrollHorz);
            else item->SetScrollPos(number,SakuraGL::SGLSprite::scrollHorz);
        }
    }
    return eslErrSuccess;
}

ESLError ECSSprite::CommitAllReference(ECSContext& context) {
    GraphicsLock lock;
    if(!legacyRestorePending_)return eslErrSuccess;
    if(legacyRestoreCommitting_)return ESLErrorMsg("Saved Sprite graphics dependency cycle cannot be reconstructed");
    struct Committing {bool& flag;Committing(bool& f):flag(f){flag=true;}~Committing(){flag=false;}} committing(legacyRestoreCommitting_);
    if(const auto error=ECSResource::CommitAllReference(context))return error;
    for(auto& ref:legacyReferences_)if(const auto error=ref.CommitAllReference(context))return error;
    if(const auto error=legacyItemState_.CommitAllReference(context))return error;
    auto* parent=ESLTypeCast<ECSSprite>(legacyReferences_[2].m_pRef);
    auto* image=ESLTypeCast<ECSResource>(legacyReferences_[0].m_pRef);
    auto* alpha=ESLTypeCast<ECSResource>(legacyReferences_[1].m_pRef);
    for(auto* dependency:{ESLTypeCast<ECSSprite>(image),ESLTypeCast<ECSSprite>(alpha),parent})
        if(dependency&&dependency->IsLegacyRestorePending())if(const auto error=dependency->CommitAllReference(context))return error;
    if(!legacyPage_.IsEmpty()) {
        auto* manager=ESLTypeCast<ECSResourceManager>(legacyReferences_[3].m_pRef);
        auto* formed=ESLTypeCast<SakuraGL::SGLSpriteFormed>(sprite_.get());
        if(!manager||!formed||!manager->GetSkin()||!manager->GetSkin()->GetFormAs(legacyPage_))
            return ESLErrorMsg("Saved Sprite skin page or ResourceManager is unavailable");
        formSkin_=manager->GetSkin();
        if(formSkin_->BuildFormedPage(*formed,legacyPage_))return eslErrGeneral;
    }
    if(const auto error=CommitLegacySpriteDrawing(context,*this,image,legacyImageFrame_,legacyImageView_,alpha))return error;
    if(const auto error=RestoreLegacyItems())return error;
    if(parent) {
        for(auto* ancestor=&parent->NativeSprite();ancestor;ancestor=ancestor->GetParent())
            if(ancestor==sprite_.get())return ESLErrorMsg("Saved Sprite parent graph contains a cycle");
        parent->NativeSprite().AddChild(sprite_.get());
    }
    if(legacyReferences_[4].m_pRef) {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(this));args.Add(new ECSReference(legacyReferences_[4].m_pRef));bool handled=false;
        if(const auto error=HandleLegacySpriteDraw(context,*this,L"AttachToneFilter",args,handled))return error;
        auto* result=context.PopObject();INT64 status=0;
        const auto error=result?result->OperateInteger(status):eslErrGeneral;context.delete_CSObject(result);
        if(error||status)return error?error:ESLError(status);
    }
    if(const auto error=CommitLegacySpriteCallbacks(*this,context))return error;
    legacyRestorePending_=false;
    return eslErrSuccess;
}
