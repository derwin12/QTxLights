#include "MainWindow.h"
#include "SequenceGrid.h"
#include "XsqFile.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>

static const QStringList SAMPLE_MODELS = {
    "Mega Tree", "Matrix 1", "Arches 1", "Arches 2",
    "Arches 3",  "Arches 4", "Roofline", "Candy Canes",
    "Stars",     "Spinner",  "Icicles",  "Window Frames",
};

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("QTxLights");
    resize(1280, 720);

    setupMenus();
    setupToolBar();
    setupCentralWidget();
    populateSampleData();

    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(new QLabel("Frame: 0 / 400  |  FPS: 20"));
}

// ---------------------------------------------------------------------------
// setupMenus
// ---------------------------------------------------------------------------
void MainWindow::setupMenus()
{
    QMenuBar* mb = menuBar();

    QMenu* file = mb->addMenu("&File");
    file->addAction("&New Sequence",     QKeySequence::New,  this, &MainWindow::onNewSequence);
    file->addAction("&Open Sequence...", QKeySequence::Open, this, &MainWindow::onOpenSequence);
    file->addSeparator();
    file->addAction("&Save",             QKeySequence::Save, this, &MainWindow::onSaveSequence);
    file->addAction("Save &As...");
    file->addSeparator();
    file->addAction("&Preferences...");
    file->addSeparator();
    file->addAction("E&xit", QKeySequence::Quit, this, &QWidget::close);

    QMenu* edit = mb->addMenu("&Edit");
    edit->addAction("&Undo",       QKeySequence::Undo);
    edit->addAction("&Redo",       QKeySequence::Redo);
    edit->addSeparator();
    edit->addAction("Cu&t",        QKeySequence::Cut);
    edit->addAction("&Copy",       QKeySequence::Copy);
    edit->addAction("&Paste",      QKeySequence::Paste);
    edit->addAction("&Delete",     QKeySequence::Delete);
    edit->addSeparator();
    edit->addAction("Select &All", QKeySequence::SelectAll);

    QMenu* seq = mb->addMenu("&Sequence");
    seq->addAction("Sequence &Settings...");
    seq->addAction("Edit &Timing...");
    seq->addSeparator();
    seq->addAction("&Play",  QKeySequence(Qt::Key_Space));
    seq->addAction("&Stop",  QKeySequence(Qt::Key_Escape));
    seq->addAction("Pa&use");

    QMenu* view = mb->addMenu("&View");
    view->addAction("Zoom &In",  QKeySequence::ZoomIn,  this, &MainWindow::onZoomIn);
    view->addAction("Zoom &Out", QKeySequence::ZoomOut, this, &MainWindow::onZoomOut);
    view->addSeparator();
    view->addAction("Toggle &Model List");
    view->addAction("Toggle &Preview");

    QMenu* help = mb->addMenu("&Help");
    help->addAction("&About QTxLights...", this, &MainWindow::onAbout);
}

// ---------------------------------------------------------------------------
// setupToolBar
// ---------------------------------------------------------------------------
void MainWindow::setupToolBar()
{
    QToolBar* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->addAction("New",   this, &MainWindow::onNewSequence);
    tb->addAction("Open",  this, &MainWindow::onOpenSequence);
    tb->addAction("Save",  this, &MainWindow::onSaveSequence);
    tb->addSeparator();
    tb->addAction("|<<");
    tb->addAction("Play");
    tb->addAction("Stop");
    tb->addAction("Pause");
    tb->addSeparator();
    tb->addAction("Zoom In",  this, &MainWindow::onZoomIn);
    tb->addAction("Zoom Out", this, &MainWindow::onZoomOut);
}

// ---------------------------------------------------------------------------
// setupCentralWidget
// ---------------------------------------------------------------------------
void MainWindow::setupCentralWidget()
{
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // Left panel — row/track list
    QWidget*     left = new QWidget(splitter);
    QVBoxLayout* lv   = new QVBoxLayout(left);
    lv->setContentsMargins(0, 0, 0, 0);
    lv->setSpacing(0);

    m_listHeader = new QLabel("Models", left);
    m_listHeader->setAlignment(Qt::AlignCenter);
    m_listHeader->setStyleSheet("background:#3c3c3c;color:white;padding:4px;font-weight:bold;");
    lv->addWidget(m_listHeader);

    m_modelList = new QListWidget(left);
    m_modelList->setFixedWidth(160);
    m_modelList->setStyleSheet("background:#2d2d2d;color:#ccc;border:none;");
    lv->addWidget(m_modelList);

    // Right panel — sequence grid
    m_grid = new SequenceGrid(splitter);

    splitter->addWidget(left);
    splitter->addWidget(m_grid);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);
}

// ---------------------------------------------------------------------------
// populateSampleData
// ---------------------------------------------------------------------------
void MainWindow::populateSampleData()
{
    m_listHeader->setText("Models");
    m_modelList->clear();
    m_modelList->addItems(SAMPLE_MODELS);
    m_grid->loadSampleData();
}

// ---------------------------------------------------------------------------
// onNewSequence
// ---------------------------------------------------------------------------
void MainWindow::onNewSequence()
{
    statusBar()->showMessage("New sequence");
}

// ---------------------------------------------------------------------------
// onOpenSequence
//
// Opens a file-picker filtered to .xsq files, parses the chosen file, and
// loads the timing tracks into the grid.
// ---------------------------------------------------------------------------
void MainWindow::onOpenSequence()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open xLights Sequence"),
        QString(),
        tr("xLights Sequences (*.xsq);;All Files (*)")
    );

    if (path.isEmpty())
        return;

    const XsqSequence seq = parseXsqFile(path);

    if (!seq.isValid()) {
        QMessageBox::warning(this, tr("Load Error"), seq.parseError);
        return;
    }

    // Update the grid.
    m_grid->loadFromXsq(seq);

    // Update the left-panel list to show timing track names.
    m_listHeader->setText("Timing Tracks");
    m_modelList->clear();
    for (const TimingTrack& track : seq.timingTracks)
        m_modelList->addItem(track.name);

    // Update the window title to reflect the loaded song or filename.
    const QString title = seq.song.isEmpty()
        ? QFileInfo(path).fileName()
        : seq.song;
    setWindowTitle(QStringLiteral("QTxLights \u2014 ") + title);

    // Report a concise summary in the status bar.
    const int trackCount = seq.timingTracks.size();
    m_statusLabel->setText(
        QStringLiteral("Loaded: %1 timing track%2  |  %3 frames @ %4 ms/frame")
            .arg(trackCount)
            .arg(trackCount == 1 ? "" : "s")
            .arg(seq.totalFrames())
            .arg(seq.frameDurationMs)
    );
}

// ---------------------------------------------------------------------------
// onSaveSequence
// ---------------------------------------------------------------------------
void MainWindow::onSaveSequence()
{
    statusBar()->showMessage("Save — not yet implemented");
}

// ---------------------------------------------------------------------------
// onZoomIn / onZoomOut
// ---------------------------------------------------------------------------
void MainWindow::onZoomIn()  { m_grid->zoom(1.25); }
void MainWindow::onZoomOut() { m_grid->zoom(0.8);  }

// ---------------------------------------------------------------------------
// onAbout
// ---------------------------------------------------------------------------
void MainWindow::onAbout()
{
    QMessageBox::about(this, "About QTxLights",
        "<b>QTxLights</b> v0.1<br><br>"
        "Qt6 proof of concept for xLights.");
}
