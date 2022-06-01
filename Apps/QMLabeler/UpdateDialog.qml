import QtQuick 2.0
import QtQuick.Controls 2.5

Dialog {
    objectName: "UpdateDialog"
    width: 400
    height: 1.5*column.height + acceptB.height + acceptB.anchors.topMargin + 2*padding

    function show(_appname, _version, _changelog) {
        title = qsTr("Загружено новое обновление");
        versionL.text = qsTr("Версия: ") + _version
        changelogL.text = qsTr("Что нового: ") + _changelog
        open();
    }

    Column {
        id: column
        width: parent.width
        Label {
            id: versionL
            width: parent.width
            wrapMode: Text.WordWrap
        }
        Label {
            id: changelogL
            width: parent.width
            wrapMode: Text.WordWrap
        }
        Label {
            wrapMode: Text.WordWrap
            width: parent.width
            text: qsTr("Перед обновлением программа будет закрыта. Вы согласны?")
        }
    }
    Button {
        id: acceptB
        text: qsTr("Обновить")
        onClicked: accepted()
        anchors.top: column.bottom
        anchors.topMargin: 10
        anchors.right: rejectB.left
        anchors.rightMargin: 10
    }
    Button {
        id: rejectB
        text: qsTr("Отмена")
        onClicked: close()
        anchors.top: column.bottom
        anchors.topMargin: 10
        anchors.right: column.right
        anchors.rightMargin: 10
    }

}
