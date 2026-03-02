#pragma once
#include <QMainWindow>

class SequenceGrid;
class QListWidget;
class QSplitter;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void setupMenus();
    void setupToolBar();
    void setupCentralWidget();
    void populateSampleData();

    SequenceGrid* m_grid;
    QListWidget*  m_modelList;
    QLabel*       m_statusLabel;

private slots:
    void onNewSequence();
    void onOpenSequence();
    void onSaveSequence();
    void onAbout();
    void onZoomIn();
    void onZoomOut();
};
