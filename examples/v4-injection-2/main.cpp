#include <QtCore>
#include <QtGui>
#include <QtQml>
#include <QtQuick>

#include <private/qv4engine_p.h>
#include <private/qv8engine_p.h>

Q_DECLARE_LOGGING_CATEGORY(MY_LOG_CATEGORY)

Q_LOGGING_CATEGORY(MY_LOG_CATEGORY, "com.kdab.log")

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(const QLoggingCategory &category, QQmlEngine *engine, QObject *parent = 0)
        : QObject(parent),
          m_category(category),
          m_engine(engine)
    {}

    Q_INVOKABLE void log(const QString &message)
    {
        const QV4::StackFrame frame = QV8Engine::getV4(m_engine)->currentStackFrame();

        QMessageLogger(qPrintable(frame.source),
                       frame.line,
                       qPrintable(frame.function),
                       m_category.categoryName()).debug("%s", qPrintable(message));
    }

private:
    const QLoggingCategory &m_category;
    QQmlEngine *m_engine;
};

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qSetMessagePattern("[%{if-debug}D%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - Category: %{if-category}%{category}%{endif} - %{message}");

    QQuickView view;
    Logger logger(MY_LOG_CATEGORY(), view.engine());
    view.rootContext()->setContextProperty("_logger", &logger);

    view.setSource(QUrl::fromLocalFile("../Logging.qml"));

    view.show();

    return app.exec();
}

#include "main.moc"
