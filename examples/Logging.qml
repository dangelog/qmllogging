import QtQuick 2.2

Item {
    width: 400
    height: 400

    Component.onCompleted: {
        console.log("Hello from console");
        console.warn("Warning from console");

        if (typeof _logger != 'undefined')
            _logger.log("Hello from logger");
    }
}
