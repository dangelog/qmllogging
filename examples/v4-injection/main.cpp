#include <QtCore>
#include <QtGui>
#include <QtQuick>

#include <private/qv4global_p.h>
#include <private/qv4object_p.h>
#include <private/qv4context_p.h>
#include <private/qv4engine_p.h>
#include <private/qv8engine_p.h>

Q_DECLARE_LOGGING_CATEGORY(MY_LOG_CATEGORY)

Q_LOGGING_CATEGORY(MY_LOG_CATEGORY, "com.kdab.log")

struct Logger : QV4::Object
{
    Logger(const QLoggingCategory &loggingCategory, QV4::ExecutionEngine *v4);
    const QLoggingCategory &category() const;

private:
    enum LogType {
        DebugLog,
        WarningLog
    };

    static QV4::ReturnedValue method_log(LogType type, QV4::CallContext *ctx);

    static QV4::ReturnedValue method_debug(QV4::CallContext *ctx);
    static QV4::ReturnedValue method_warning(QV4::CallContext *ctx);

    const QLoggingCategory &m_loggingCategory;
};

Logger::Logger(const QLoggingCategory &loggingCategory, QV4::ExecutionEngine *v4)
    : QV4::Object(v4)
    , m_loggingCategory(loggingCategory)
{
    QV4::Scope scope(v4);
    QV4::ScopedObject protectThis(scope, this);
    Q_UNUSED(protectThis);

    defineDefaultProperty(QStringLiteral("log"), method_debug);
    defineDefaultProperty(QStringLiteral("debug"), method_debug);
    defineDefaultProperty(QStringLiteral("warn"), method_warning);
}

const QLoggingCategory &Logger::category() const
{
    return m_loggingCategory;
}

QV4::ReturnedValue Logger::method_log(LogType type, QV4::CallContext *ctx)
{
    QV4::ExecutionEngine *v4ee = ctx->engine;

    QString loggedMessage;

    for (int i = 0; i < ctx->callData->argc; ++i) {
        if (i != 0)
            loggedMessage.append(QLatin1Char(' '));

        if (ctx->callData->args[i].asArrayObject())
            loggedMessage.append(QStringLiteral("[") + ctx->callData->args[i].toQStringNoThrow() + QStringLiteral("]"));
        else
            loggedMessage.append(ctx->callData->args[i].toQStringNoThrow());
    }

    QV4::StackFrame frame = v4ee->currentStackFrame();
    const QByteArray sourceFile = frame.source.toUtf8();
    const QByteArray sourceFunction = frame.function.toUtf8();
    const int line = frame.line;

    Logger *thisObj = static_cast<Logger *>(ctx->callData->thisObject.toObject(ctx));
    const QLoggingCategory &category = thisObj->category();

    QMessageLogger logger(sourceFile.constData(), line, sourceFunction.constData(), category.categoryName());

    switch (type) {
    case DebugLog:
        if (category.isDebugEnabled())
            logger.debug("%s", qPrintable(loggedMessage));
        break;
    case WarningLog:
        if (category.isWarningEnabled())
            logger.warning("%s", qPrintable(loggedMessage));
        break;
    }

    return QV4::Encode::undefined();
}

QV4::ReturnedValue Logger::method_debug(QV4::CallContext *ctx)
{
    return method_log(DebugLog, ctx);
}

QV4::ReturnedValue Logger::method_warning(QV4::CallContext *ctx)
{
    return method_log(WarningLog, ctx);
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qSetMessagePattern("[%{if-debug}D%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - Category: %{if-category}%{category}%{endif} - %{message}");

    QQuickView view;

    {
        QQmlEngine *engine = view.engine();
        QV4::ExecutionEngine *v4ee = QV8Engine::getV4(engine);

        QV4::Scope scope(v4ee);
        QV4::ScopedValue logger(scope, new (v4ee->memoryManager) Logger(MY_LOG_CATEGORY(), v4ee));
        v4ee->globalObject->defineDefaultProperty(QStringLiteral("_logger"), logger);
    }

    view.setSource(QUrl::fromLocalFile("../Logging.qml"));

    view.show();

    return app.exec();
}
