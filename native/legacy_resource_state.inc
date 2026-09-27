// Win32 GLS3 Resource record: seven LE DWORDs, two float32s, counted UTF-16
// filename and the hidden attached-sound Reference. No host pointer is saved.
ESLError ECSResource::Save(ESLFileObject &file, ECSContext &context) {
    using namespace StudySteadyLegacyWire;
    const uint32_t kind = m_image ? 1 : m_sound ? 2 : 0;
    if (kind && m_wstrFileName.IsEmpty() && !(kind == 2 && m_refAttachSound.m_pRef))
        return ESLErrorMsg("Resource cannot save anonymous or generated media without a restorable source");
    if (m_restorePlayback)
        return ESLErrorMsg("Resource cannot save before its loaded references and playback have been committed");
    LegacyVolumePoint volume;
    {
        std::lock_guard<std::mutex> envelopeLock(m_envelopeMutex);
        std::lock_guard<std::mutex> volumeLock(m_volumeMutex);
        volume = {m_volume[0].load(), m_volume[1].load()};
        if (m_envelope && m_envelope->Pending()) volume = m_envelope->Endpoint();
    }
    int type = m_nPlayType.load();
    uint32_t start = m_nStartPos;
    if (type != ptfNothing) {
        if (!IsPlaying()) type = ptfNothing;
        if (m_sound && !m_nRepeatPlaying) {
            const auto position = m_sound->GetPosition();
            if (position > UINT32_MAX) return eslErrNotSupported;
            start = uint32_t(position);
        }
    }
    uint8_t header[36];
    const uint32_t fields[] = {kind, uint32_t(type), m_nThreshold, m_nRewindPos,
        start, m_nEndPos, m_nRepeatPlaying};
    for (unsigned i = 0; i < 7; ++i) Write32(header + i*4, fields[i]);
    uint32_t bits;
    std::memcpy(&bits, &volume.left, 4); Write32(header+28, bits);
    std::memcpy(&bits, &volume.right, 4); Write32(header+32, bits);
    // Validate the reference graph before writing the media header.
    if (const auto error = m_refAttachSound.CommitGlobalReference(context)) return error;
    if (file.Write(header, sizeof(header)) != sizeof(header) || !WriteWideString(file, m_wstrFileName))
        return eslErrGeneral;
    return m_refAttachSound.Save(file, context);
}

ESLError ECSResource::Load(ESLFileObject &file, ECSContext &context) {
    using namespace StudySteadyLegacyWire;
    uint8_t header[36];
    if (file.Read(header, sizeof(header)) != sizeof(header)) return eslErrGeneral;
    const uint32_t kind = Read32(header), threshold = Read32(header+8);
    const int32_t type = int32_t(Read32(header+4));
    const uint32_t rewind = Read32(header+12), start = Read32(header+16);
    const uint32_t end = Read32(header+20), repeat = Read32(header+24);
    LegacyVolumePoint volume;
    uint32_t bits = Read32(header+28); std::memcpy(&volume.left, &bits, 4);
    bits = Read32(header+32); std::memcpy(&volume.right, &bits, 4);
    if (kind > 3 || (type < ptfNothing || type >= ptfMax) || repeat > 2 ||
        !std::isfinite(volume.left) || !std::isfinite(volume.right) || volume.left < 0 || volume.right < 0)
        return eslErrInvalidParam;
    if (kind == 3) return ESLErrorMsg("Saved Resource MIDI has no Android playback backend");
    EWideString path;
    if (!ReadWideString(file, path)) return eslErrGeneral;
    if ((!kind && !path.IsEmpty()) || (kind == 1 && path.IsEmpty()))
        return ESLErrorMsg("Saved Resource image has no restorable filename");
    // Qualified release only clears the Resource portion; derived Sprite::Load
    // owns restoration of its native image references, forms and hierarchy.
    ECSResource::Release();
    if (!path.IsEmpty()) {
        const auto error = kind == 1 ? LoadImageFile(path, &context) : LoadSoundFile(path, threshold, &context);
        if (error) return error; // A missing file cannot be called a restored resource.
    }
    if (const auto error = m_refAttachSound.Load(file, context)) return error;
    m_nThreshold = threshold; m_nRewindPos = rewind; m_nStartPos = start;
    m_nEndPos = end; m_nRepeatPlaying = repeat; m_nPlayType = type;
    if (const auto error = SetVolume(volume.left, volume.right)) return error;
    m_restoreResourceKind = kind;
    m_restorePlayback = true;
    return eslErrSuccess;
}
