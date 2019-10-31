import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480

    function setAppTitle(_title) { window.title = _title  }
    function setAppAbout(_appname, _appversion, _description, _disclaimer, _authors) {
        appname.text = _appname + " v" + _appversion;
        description.text = _description;
        disclaimer.text = _disclaimer;
        authors.text = _authors;
    }    
    function openImage(_filename) { image.source =_filename }
    function showInfo(_info) { infoLabel.text = _info }
    function showError(_error) { errorLabel.text = _error }
    function dropImage() {image.source = ""}
    function showUpdateDialog(_appname,_version,_changelog) { updatedialog.show(_appname,_version,_changelog) }

    UpdateDialog {
        id: updatedialog
        x: (window.width - width)   / 2
        y: (window.height - height) /2
    }

    ProgressBar {
        z: 1
        width: window.width*0.9
        x: (window.width - width) / 2
        y: window.height / 5
        value: customsettings.updtdownloadprogress
        visible: value > 0.01 && value < 0.99
        Label {
            text: qsTr("Загрузка обновлений:")
            anchors.bottom: parent.top
        }
    }

    Drawer {
        id: drawer
        width: 0.6 * window.width
        height: window.height

        Column {
            id: topColumn
            width: parent.width * 0.95
            y: 10
            x: (parent.width - width)/2

            /*Label {
                width: parent.width
                text: qsTr("Выберите язык интерфейса:")
                elide: Text.ElideRight
            }
            ComboBox {
                flat: true
                width: parent.width
                model: ["Русский", "English"]
                currentIndex: settings.language === "Русский" ? 0 : 1
                onCurrentIndexChanged: settings.language = model[currentIndex]
            }*/

            Label {
                width: parent.width
                text: qsTr("Укажите директорию с исходниками:")
                elide: Text.ElideRight
            }
            Pane {
                background.opacity: 0
                width: parent.width
                height: inputdirTF.height*1.4
                TextField {
                    readOnly: true
                    id: inputdirTF
                    width:  parent.width*0.8
                    text: customsettings.inputdir
                    FileDialog {
                        id: inputdirDialog
                        selectFolder: true
                        folder: customsettings.inputdir
                        onAccepted: customsettings.inputdir = inputdirDialog.folder.toString()
                    }
                }
                Button {
                    text: qsTr("...")
                    flat: true
                    onClicked: inputdirDialog.open()
                    anchors.right: parent.right
                }
            }

            Label {
                width: parent.width
                text: qsTr("Укажите директорию для размеченных изображений:")
                elide: Text.ElideRight
            }
            Pane {
                background.opacity: 0
                width: parent.width
                height: outputdirTF.height*1.4
                TextField {
                    readOnly: true
                    id: outputdirTF
                    width:  parent.width*0.8
                    text: customsettings.outputdir
                    FileDialog {
                        id: outputdirDialog
                        selectFolder: true
                        folder: customsettings.outputdir
                        onAccepted: {
                            customsettings.outputdir = outputdirDialog.folder.toString()
                        }
                    }
                }
                Button {
                    text: qsTr("...")
                    flat: true
                    onClicked: outputdirDialog.open()
                    anchors.right: parent.right
                }
            }

            Label {
                width: parent.width
                text: qsTr("Адрес сервера для проверки обновлений:")
                elide: Text.ElideRight
            }
            TextField {
                x: (parent.width - width)/2
                horizontalAlignment: Text.AlignHCenter
                text: customsettings.updtsrvaddr
                onTextChanged: customsettings.updtsrvaddr = text
            }
            Label {
                width: parent.width
                text: qsTr("Порт сервера для проверки обновлений:")
                elide: Text.ElideRight
            }
            SpinBox {
                from: 0
                to: 65535
                x: (parent.width - width)/2
                value: customsettings.updtsrvport
                onValueChanged: customsettings.updtsrvport = value
                editable: true
            }
        }

        Column {
            width: topColumn.width
            x: topColumn.x
            anchors.bottom: parent.bottom
            Text {
                id: appname
                padding: 5
                font.bold: true
                width: parent.width
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                opacity: 0.5
            }
            Text {
                id: description
                padding: 5
                width: parent.width
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                opacity: 0.5
            }
            Text {
                id: disclaimer
                padding: 5
                width: parent.width
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                opacity: 0.5
            }
            Text {
                id: authors
                padding: 5
                font.bold: true
                width: parent.width
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                opacity: 0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }

    Pane {
        width: window.width
        height: window.height

        Label {
            id: infoLabel
            text: ""
            width: parent.width
            wrapMode: Text.WordWrap
            anchors.bottom: parent.bottom
        }

        Image {
            id: image
            height: parent.height - 1.5*infoLabel.height
            width: parent.width*0.7
            fillMode: Image.PreserveAspectFit

            Label {
                opacity: 0.5
                text: qsTr("Приложение предназначено для ручной разметки изображений на классы.
\nДля начала работы, вы должны указать в меню две директории. Первая, директория с файлами исходных изображений. Вторая, директория с субдиректориями, имена которых обозначают классы к одному из которых должно быть отнесено каждое изображение.
\nПриложение копирует файлы из директории с исходниками в директорию с разметкой, поэтому исходные файлы не удаляются.
\nПри работе в одной и той же директории с исходными файлами ведётся история размеки изображений, поэтому, после закрытия программы, при возобновлении работы над размекой вы продолжаете с того файла на котором закончили.
\nКлассы меток отображаются справа. Для маркировки изображения просто нажмите подходящий чек-бокс")
                //horizontalAlignment: Text.AlignHCenter
                //anchors.centerIn: parent
                width: parent.width
                wrapMode: Text.WordWrap
                visible: !image.source.toString()
            }
        }

        GridView {
            height: image.height
            width: parent.width - image.width
            anchors.left: image.right
            clip: true
            model: labels
            cellWidth: 175
            cellHeight: 50
            ScrollIndicator.vertical: ScrollIndicator { }
            delegate: Pane {
                width: parent.width
                padding: 5
                CheckBox {
                    text: name
                    checked: modelData.checked
                    onCheckedChanged: modelData.checked = checked
                }
            }
            Label {
                id: errorLabel
                color: "red"
                anchors.fill: parent
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                //visible: parent.count > 0
            }
        }
    }
}
