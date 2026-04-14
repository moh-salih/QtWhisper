#include <QMetaType>
#include <QtWhisper/Session.h>

namespace QtWhisper {

Session::Session(QObject *parent): QObject(parent) {}

Session::~Session() {
    if (mWorkerThread) {
        mWorkerThread->quit();
        mWorkerThread->wait();
    }
}

void Session::initialize(IEngine* engine) {
    if (mWorkerThread) return;

    qRegisterMetaType<std::vector<float>>("std::vector<float>");
    
    mEngine = engine;
    mWorkerThread = new QThread(this);
    mEngine->setParent(nullptr);
    mEngine->moveToThread(mWorkerThread);
    mEngine->setConfig(mConfig);

    connect(mEngine, &IEngine::transcriptionSegmentReady, this, &Session::onPartialResult);
    
    connect(mEngine, &IEngine::modelStatusChanged, this, [this](Status s){
        if (mStatus != s) {
            mStatus = s;
            emit statusChanged();
        }
    });

    connect(mWorkerThread, &QThread::finished, mEngine, &QObject::deleteLater);
    mWorkerThread->start();
}

void Session::setConfig(const Config& config) {
    *mConfig = config;
}

void Session::loadModel() {
    if (!mConfig || mConfig->modelPath.isEmpty()) return;
    QMetaObject::invokeMethod(mEngine, "loadModel");
}

void Session::unloadModel() {
    stopInference();
    QMetaObject::invokeMethod(mEngine, "unloadModel");
}

void Session::processAudioWindow(const std::vector<float> &samples) {
    if (mStatus != Status::Ready || samples.empty()) return;
    QMetaObject::invokeMethod(mEngine, "processWindow", Q_ARG(std::vector<float>, samples));
}

void Session::startInference() {
    if (mStatus != Status::Ready) return;
    QMetaObject::invokeMethod(mEngine, "reset");
}

void Session::stopInference() {
    QMetaObject::invokeMethod(mEngine, "stop");
}

void Session::onPartialResult(const QString& text) {
    if (text.isEmpty()) return;
    mTranscription += (mTranscription.isEmpty() ? "" : " ") + text;
    emit transcriptionChanged(mTranscription);
    emit partialResult(text);
}

void Session::clear() {
    mTranscription.clear();
    emit transcriptionChanged(mTranscription);
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
