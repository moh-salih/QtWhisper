#include <QMetaType>
#include <QDebug>
#include <QtWhisper/Session.h>

namespace QtWhisper {

Session::Session(QObject *parent) : QObject(parent) {}

Session::~Session() {
    if (mWorkerThread) {
        mWorkerThread->quit();
        mWorkerThread->wait();
    }
}

void Session::initialize(IEngine *engine) {
    Q_ASSERT_X(!mWorkerThread, "Session::initialize",
               "initialize() called more than once — Session takes ownership on first call");

    qRegisterMetaType<std::vector<float>>("std::vector<float>");
    qRegisterMetaType<QtWhisper::Status>("QtWhisper::Status");

    mEngine = engine;
    mWorkerThread = new QThread(this);
    mEngine->setParent(nullptr);
    mEngine->moveToThread(mWorkerThread);
    mEngine->setConfig(mConfig);

    connect(mEngine, &IEngine::segmentTranscribed,
            this,    &Session::onSegmentTranscribed);
    connect(mEngine, &IEngine::statusChanged,
            this,    &Session::onStatusChanged);
    connect(mEngine, &IEngine::processingBusyChanged,
            this,    &Session::onProcessingBusyChanged);
    connect(mEngine, &IEngine::errorEncountered,
            this,    &Session::errorEncountered);
    connect(mEngine, &IEngine::reloadRequired, this, &Session::reloadRequired);

    connect(mWorkerThread, &QThread::finished, mEngine, &QObject::deleteLater);
    mWorkerThread->start();
}

void Session::setConfig(const Config &config) {
    *mConfig = config;

    // If the worker is already live, push the updated config across to the
    // engine thread. Config changes only take effect on the next loadModel()
    // call — the running model is not reloaded.
    if (mWorkerThread)
        QMetaObject::invokeMethod(mEngine, "setConfig",
                                  Q_ARG(QSharedPointer<Config>, mConfig));
}

void Session::loadModel() {
    if (!mConfig || mConfig->modelPath.isEmpty()) return;
    QMetaObject::invokeMethod(mEngine, "loadModel");
}

void Session::unloadModel() {
    stopInference();
    QMetaObject::invokeMethod(mEngine, "unloadModel");
}

void Session::reloadModel() {
    QMetaObject::invokeMethod(mEngine, "reloadModel");
}

void Session::processAudioWindow(const std::vector<float> &samples) {
    if (mStatus != Status::Ready || samples.empty()) return;

    if (mIsProcessing) {
        emit audioWindowDropped();
        return;
    }

    QMetaObject::invokeMethod(mEngine, "processWindow",
                              Q_ARG(std::vector<float>, samples));
}

void Session::resumeInference() {
    QMetaObject::invokeMethod(mEngine, "reset");
}

void Session::stopInference() {
    QMetaObject::invokeMethod(mEngine, "stop");
}

void Session::clear() {
    mTranscription.clear();
    emit transcriptionChanged(mTranscription);
}

void Session::onSegmentTranscribed(const QString &segment) {
    if (segment.isEmpty()) return;
    mTranscription += (mTranscription.isEmpty() ? "" : " ") + segment;
    emit transcriptionChanged(mTranscription);
    emit segmentTranscribed(segment);
}

void Session::onProcessingBusyChanged(bool isProcessing) {
    if (mIsProcessing == isProcessing) return;
    mIsProcessing = isProcessing;
    emit processingBusyChanged(isProcessing);
}

void Session::onStatusChanged(QtWhisper::Status status) {
    if (mStatus == status) return;
    mStatus = status;
    emit statusChanged(mStatus);
}

QString Session::statusText() const {
    switch (mStatus) {
        case Status::Idle:    return tr("No Model");
        case Status::Loading: return tr("Loading...");
        case Status::Ready:   return tr("Ready");
        case Status::Error:   return tr("Error");
        default:              return tr("Unknown");
    }
}

} // namespace QtWhisper
