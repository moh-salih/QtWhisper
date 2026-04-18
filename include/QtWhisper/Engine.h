#pragma once
#include <whisper.h>
#include <atomic>
#include <string>
#include <QtWhisper/IEngine.h>
#include <QtWhisper/Types.h>

namespace QtWhisper {

    class Engine : public IEngine {
        Q_OBJECT
    public:
        explicit Engine(QObject *parent = nullptr);
        ~Engine() override;

    public slots:
        void setConfig(QSharedPointer<Config> config) override;
        void loadModel() override;
        void unloadModel() override;
        void processWindow(const std::vector<float> &samples) override;
        void stop() override;
        void reset() override;

    private:
        static bool abort_callback(void *user_data);

        whisper_context        *m_ctx = nullptr;
        std::atomic<bool>       m_abortRequested{false};
        QSharedPointer<Config>  mConfig;
        std::string             mLanguageStd;
    };

} // namespace QtWhisper
