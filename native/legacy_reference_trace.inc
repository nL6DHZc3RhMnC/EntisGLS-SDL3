    struct LegacyReferenceTraceStep { ECSObject* parent; ECSObject* target; int slot; };
    std::vector<LegacyReferenceTraceStep> traceSteps;
    auto reportMissingReference = [&](const char* reason, int failedStep) {
        std::string path;
        for (size_t i = 0; i < m_dimIndex.GetSize() && i < 512; ++i)
            path += (i ? "," : "") + std::to_string(m_dimIndex.GetAt(i));
        if (m_dimIndex.GetSize() > 512) path += ",...";
        int stackSlot = -1;
        for (size_t i = 0; i < context.m_stack.m_varArray.GetSize(); ++i)
            if (context.m_stack.m_varArray.GetAt(i) == this) { stackSlot = int(i); break; }
        study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady",
            "Primary reference missing ref=%p mode=%d own=%p dimensions=%u path=[%s] failedStep=%d reason=%s directStackSlot=%d cast=%d/%d/%d",
            this, int(m_omBaseMode), m_pOwnObj, m_dimIndex.GetSize(), path.c_str(), failedStep, reason, stackSlot,
            m_iVarOffset, m_nVarBounds, m_iFuncOffset);
        for (size_t i = 0; i < traceSteps.size(); ++i) {
            const auto& step = traceSteps[i];
            study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady",
                "Primary reference step=%zu parent=%p type=%ls slot=%d target=%p targetType=%ls",
                i, step.parent, step.parent ? step.parent->GetTypeName() : L"<null>", step.slot,
                step.target, step.target ? step.target->GetTypeName() : L"<null>");
        }
    };
