#include <QtWhisper/Engine.h>
#include <QtWhisper/Types.h>
#include <QDebug>

namespace QtWhisper {

Engine::Engine(QObject *parent) : IEngine(parent) {}

Engine::~Engine() {
    if (m_ctx) whisper_free(m_ctx);
}

bool Engine::requiresReload(const Config& next, const Config& current) const {
    return next.modelPath != current.modelPath
        || next.useGpu    != current.useGpu;
}

void Engine::setConfig(QSharedPointer<Config> config) {
    const bool needsReload = mConfig && requiresReload(*config, *mConfig);
    mConfig      = config;
    mLanguageStd = config->language.toStdString();

    if (needsReload && m_ctx != nullptr) {
        if (mConfig->autoReload)
            reloadModel();
        else
            emit reloadRequired();
    }
}

void Engine::reloadModel() {
    unloadModel();
    loadModel();
}

bool Engine::abort_callback(void *user_data) {
    auto *engine = static_cast<Engine *>(user_data);
    return engine ? engine->m_abortRequested.load() : false;
}

void Engine::loadModel() {
    unloadModel();
    emit statusChanged(Status::Loading);

    if (!mConfig || mConfig->modelPath.isEmpty()) {
        qCritical() << "QtWhisper:" << errorToString(Error::ModelPathEmpty);
        emit errorOccurred(Error::ModelPathEmpty);
        emit statusChanged(Status::Error);
        return;
    }

    qInfo() << "=== WHISPER ENGINE LOADING ===";
    qInfo() << "  Model Path:"   << mConfig->modelPath;
    qInfo() << "  Use GPU:"      << mConfig->useGpu;
    qInfo() << "  Language:"     << mConfig->language;
    qInfo() << "  Auto Detect:"  << mConfig->autoDetectLanguage;
    qInfo() << "  Translate:"    << mConfig->translate;
    qInfo() << "  Max Tokens:"   << mConfig->maxTokens;
    qInfo() << "  Thread Count:" << mConfig->threadCount;
    qInfo() << "  Temperature:"  << mConfig->temperature;

    whisper_context_params params = whisper_context_default_params();
    params.use_gpu = mConfig->useGpu;

    m_ctx = whisper_init_from_file_with_params(
        mConfig->modelPath.toStdString().c_str(), params);

    if (!m_ctx) {
        qCritical() << "QtWhisper:" << errorToString(Error::ModelLoadFailed);
        emit errorOccurred(Error::ModelLoadFailed);
        emit statusChanged(Status::Error);
        return;
    }

    emit statusChanged(Status::Ready);
}

void Engine::processWindow(const std::vector<float> &samples) {
    if (!m_ctx || samples.empty()) return;

    m_abortRequested.store(false);
    emit processingBusyChanged(true);

    whisper_full_params p = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

    p.language                 = mLanguageStd.c_str();
    p.detect_language          = mConfig->autoDetectLanguage;
    p.translate                = mConfig->translate;
    p.max_tokens               = mConfig->maxTokens;
    p.n_threads                = mConfig->threadCount;
    p.no_context               = mConfig->mode == Mode::Live;
    p.single_segment           = mConfig->mode == Mode::Live;
    p.token_timestamps         = mConfig->mode == Mode::File;
    p.temperature_inc          = mConfig->mode == Mode::Live ? 0.0f : 0.2f;
    p.print_progress           = false;
    p.temperature              = mConfig->temperature;
    p.suppress_blank           = true;
    p.abort_callback           = abort_callback;
    p.abort_callback_user_data = this;

    const int rc = whisper_full(m_ctx, p,
                                samples.data(),
                                static_cast<int>(samples.size()));

    if (!m_abortRequested.load() && rc == 0) {
        QString text;
        const int n = whisper_full_n_segments(m_ctx);
        for (int i = 0; i < n; ++i)
            text += QString::fromUtf8(whisper_full_get_segment_text(m_ctx, i));
        if (!text.trimmed().isEmpty())
            emit segmentTranscribed(text.trimmed());
    } else if (rc != 0) {
        qCritical() << "QtWhisper:" << errorToString(Error::InferenceFailed);
        emit errorOccurred(Error::InferenceFailed);
    }

    emit processingBusyChanged(false);
}

void Engine::unloadModel() {
    m_abortRequested.store(true);
    if (m_ctx) { whisper_free(m_ctx); m_ctx = nullptr; }
    emit statusChanged(Status::Idle);
}

void Engine::stop()  { m_abortRequested.store(true); }
void Engine::reset() { m_abortRequested.store(false); }

} // namespace QtWhisper
