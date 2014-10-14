#include <QtCore>
#include <QtGui>
#include <QtQml>
#include <QtQuick>

#include <private/qv4engine_p.h>
#include <private/qv8engine_p.h>

Q_DECLARE_LOGGING_CATEGORY(MY_LOG_CATEGORY)
Q_LOGGING_CATEGORY(MY_LOG_CATEGORY, "com.kdab.log")

Q_DECLARE_LOGGING_CATEGORY(CONSOLE_CATEGORY)
Q_LOGGING_CATEGORY(CONSOLE_CATEGORY, "com.kdab.log.console")

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(const QLoggingCategory &category, QQmlEngine *engine, QObject *parent = 0)
        : QObject(parent),
          m_category(category),
          m_engine(engine)
    {}

    Q_INVOKABLE void log(const QVariant &arg0, const QVariant &arg1 = QVariant(), const QVariant &arg2 = QVariant())
    {
        print<&QMessageLogger::debug>(arg0, arg1, arg2);
    }

    Q_INVOKABLE void warn(const QVariant &arg0, const QVariant &arg1 = QVariant(), const QVariant &arg2 = QVariant())
    {
        print<&QMessageLogger::warning>(arg0, arg1, arg2);
    }


private:
    static QString join()
    {
        return {};
    }

    template <typename Head, typename ... Tail>
    static QString join(const Head &head, const Tail & ... tail)
    {
        if (!head.isValid())
            return {};

        auto headString = head.toString();
        auto tailString = join(tail...);

        if (tailString.isEmpty())
            return headString;

        return headString + ", " + tailString;
    }


    typedef void (QMessageLogger::*LogFunction)(const char *, ...) const;


    template <LogFunction logFunction, typename ... Args>
    void print(const Args & ... args)
    {
        auto message = join(args...);
        auto frame = QV8Engine::getV4(m_engine)->currentStackFrame();

        (QMessageLogger(qPrintable(frame.source),
                        frame.line,
                        qPrintable(frame.function),
                        m_category.categoryName()).*logFunction)("%s", qPrintable(message));
    }


    const QLoggingCategory &m_category;
    QQmlEngine *m_engine;
};




void installLoggers(QQmlEngine *engine)
{
    // CAN'T DO THIS! Cannot modify the global object, remember?
//    auto logger = engine->newQObject(new Logger(CONSOLE_CATEGORY(), engine));
//    engine->globalObject().setProperty("_logger", logger);

    auto consoleLogger = engine->newQObject(new Logger(CONSOLE_CATEGORY(), engine));
    auto consoleObject = engine->globalObject().property("console");
    consoleObject.setProperty("log", consoleLogger.property("log"));
    consoleObject.setProperty("warn", consoleLogger.property("warn"));
}





int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qSetMessagePattern("[%{if-debug}D%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - Category: %{if-category}%{category}%{endif} - %{message}");

    QQuickView view;

    installLoggers(view.engine());

    Logger logger(MY_LOG_CATEGORY(), view.engine());
    view.rootContext()->setContextProperty("_logger", &logger);

    view.setSource(QUrl::fromLocalFile("../Logging.qml"));

    view.show();

    return app.exec();
}

#include "main.moc"
