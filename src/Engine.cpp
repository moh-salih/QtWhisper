#include <QtWhisper/Engine.h>
#include <QDebug>
#include <QThread>

namespace QtWhisper {

Engine::Engine(QObject *parent) : IEngine(parent) {}

Engine::~Engine() {
    if (m_ctx) whisper_free(m_ctx);
}

void Engine::setConfig(QSharedPointer<Config> config){
    mConfig = config;
}

bool Engine::abort_callback(void *user_data) {
    auto *engine = static_cast<Engine*>(user_data);
    return engine ? engine->m_abortRequested.load() : false;
}

void Engine::loadModel() {
    unloadModel();
    emit modelStatusChanged(Status::Loading);

    if (!mConfig || mConfig->modelPath.isEmpty()) {
        qCritical() << "QtWhisper: Model path is empty in config.";
        emit errorEncountered(tr("Model path is not configured."));
        emit modelStatusChanged(Status::Error);
        return;
    }

    qInfo() << "=== WHISPER ENGINE LOADING ===";
    qInfo() << "  Model Path:" << mConfig->modelPath;
    qInfo() << "  Use GPU:" << (mConfig->useGpu ? "true" : "false");
    qInfo() << "  Language:" << mConfig->language;
    qInfo() << "  Auto Detect:" << (mConfig->autoDetectLanguage ? "true" : "false");
    qInfo() << "  Translate:" << (mConfig->translate ? "true" : "false");
    qInfo() << "  Max Tokens:" << mConfig->maxTokens;
    qInfo() << "  Thread Count:" << mConfig->threadCount;
    qInfo() << "  Temperature:" << mConfig->temperature;

    whisper_context_params params = whisper_context_default_params();
    params.use_gpu = mConfig->useGpu;

    m_ctx = whisper_init_from_file_with_params(mConfig->modelPath.toStdString().c_str(), params);

    if (!m_ctx) {
        emit errorEncountered("Failed to load Whisper model file");
        emit modelStatusChanged(Status::Error);
        return;
    }
    emit modelStatusChanged(Status::Ready);
}

void Engine::processWindow(const std::vector<float> &samples) {
    if (!m_ctx || samples.empty()) return;

    m_abortRequested.store(false);
    emit processingBusyChanged(true);

    whisper_full_params p = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

    std::string langStd             = mConfig->language.toStdString();

    p.detect_language               = mConfig->autoDetectLanguage;
    p.language                      = langStd.c_str();
    p.translate                     = mConfig->translate;
    p.max_tokens                    = mConfig->maxTokens;
    p.n_threads                     = mConfig->threadCount;
    p.no_context                    = true;
    p.single_segment                = true;
    p.print_progress                = false;
    p.temperature                   = mConfig->temperature;
    p.abort_callback                = abort_callback;
    p.abort_callback_user_data      = this;
    p.suppress_blank                = true;

    int rc = whisper_full(m_ctx, p, samples.data(), static_cast<int>(samples.size()));

    if (!m_abortRequested.load() && rc == 0) {
        QString text;
        int n = whisper_full_n_segments(m_ctx);
        for (int i = 0; i < n; ++i) {
            text += QString::fromUtf8(whisper_full_get_segment_text(m_ctx, i));
        }
        if (!text.trimmed().isEmpty()) emit transcriptionSegmentReady(text.trimmed());
    } else if (rc != 0) {
        emit errorEncountered("Whisper inference failed");
    }

    emit processingBusyChanged(false);
}

void Engine::unloadModel() {
    m_abortRequested.store(true);
    if (m_ctx) { whisper_free(m_ctx); m_ctx = nullptr; }
    emit modelStatusChanged(Status::Idle);
}

void Engine::stop() { m_abortRequested.store(true); }
void Engine::reset() { m_abortRequested.store(false); }

} // namespace QtWhisper
