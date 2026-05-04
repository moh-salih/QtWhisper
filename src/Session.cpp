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

void Session::initialize(IEngine* engine) {
    Q_ASSERT_X(!mWorkerThread, "Session::initialize",
               "initialize() called more than once — Session takes ownership on first call");

    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<QtWhisper::Status>("QtWhisper::Status");
    qRegisterMetaType<QtWhisper::Error>("QtWhisper::Error");

    mEngine = engine;
    mWorkerThread = new QThread(this);
    mEngine->setParent(nullptr);
    mEngine->moveToThread(mWorkerThread);

    connect(mEngine, &IEngine::segmentTranscribed,  this, &Session::onSegmentTranscribed);
    connect(mEngine, &IEngine::statusChanged,       this, &Session::onStatusChanged);
    connect(mEngine, &IEngine::isProcessingChanged, this, &Session::onIsProcessingChanged);
    connect(mEngine, &IEngine::errorOccurred,       this, &Session::errorOccurred);
    connect(mEngine, &IEngine::reloadRequired,      this, &Session::reloadRequired);

    connect(mWorkerThread, &QThread::finished, mEngine, &QObject::deleteLater);
    mWorkerThread->start();
}

void Session::setConfig(const Config& config) {
    mConfig = QSharedPointer<Config>::create(config);
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

void Session::processAudioWindow(const QVector<float>& samples) {
    if (mStatus != Status::Ready || samples.isEmpty()) return;

    if (mIsProcessing) {
        emit audioWindowDropped();
        return;
    }

    QMetaObject::invokeMethod(mEngine, "processWindow",
                              Q_ARG(QVector<float>, samples));
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

void Session::onSegmentTranscribed(const QString& segment) {
    if (segment.isEmpty()) return;
    mTranscription += (mTranscription.isEmpty() ? "" : " ") + segment;
    emit transcriptionChanged(mTranscription);
    emit segmentTranscribed(segment);
}

void Session::onIsProcessingChanged(bool isProcessing) {
    if (mIsProcessing == isProcessing) return;
    mIsProcessing = isProcessing;
    emit isProcessingChanged(isProcessing);
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
