#include <QtCore>
#include <QtGui>
#include <QtQuick>

Q_DECLARE_LOGGING_CATEGORY(MY_LOG_CATEGORY)

Q_LOGGING_CATEGORY(MY_LOG_CATEGORY, "com.kdab.log")

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(const QLoggingCategory &category, QObject *parent = 0)
        : QObject(parent),
          m_category(category)
    {}

    Q_INVOKABLE void log(const QString &message)
    {
        qCDebug(m_category) << message;
    }
private:
    const QLoggingCategory &m_category;
};

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qSetMessagePattern("[%{if-debug}D%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - Category: %{if-category}%{category}%{endif} - %{message}");

    Logger logger(MY_LOG_CATEGORY());

    QQuickView view;
    view.rootContext()->setContextProperty("_logger", &logger);
    view.setSource(QUrl::fromLocalFile("../Logging.qml"));

    view.show();

    return app.exec();
}

#include "main.moc"
