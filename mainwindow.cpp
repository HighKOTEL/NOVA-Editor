#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileInfo>
#include <QCloseEvent>
#include <QTextBlock>
#include <QScrollBar>


CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), isDarkTheme(true)
{
    lineNumberArea = new LineNumberArea(this);
    
    connect(this->document(), &QTextDocument::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int){ lineNumberArea->update(); });
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::highlightCurrentLine);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::onUpdateRequest);
    
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(12);
    this->setFont(font);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    
    if (isDarkTheme) {
        painter.fillRect(event->rect(), QColor("#1e1e1e"));
    } else {
        painter.fillRect(event->rect(), QColor("#f3f3f3"));
    }
    
    QTextBlock block = this->firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) this->blockBoundingGeometry(block).translated(this->contentOffset()).top();
    int bottom = top + (int) this->blockBoundingRect(block).height();
    
    QFont font = this->font();
    font.setPointSize(10);
    painter.setFont(font);
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            
            QTextCursor cursor = this->textCursor();
            if (cursor.blockNumber() == blockNumber) {
                painter.setPen(isDarkTheme ? QColor("#569cd6") : QColor("#007acc"));
                painter.setFont(QFont(font.family(), font.pointSize(), QFont::Bold));
            } else {
                painter.setPen(isDarkTheme ? QColor("#858585") : QColor("#969696"));
                painter.setFont(font);
            }
            
            painter.drawText(0, top, lineNumberArea->width() - 5, this->fontMetrics().height(),
                           Qt::AlignRight, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + (int) this->blockBoundingRect(block).height();
        ++blockNumber;
    }
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, this->document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    int space = 15 + this->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    
    QRect cr = this->contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount);
    this->setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    if (!this->isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        
        QColor lineColor = isDarkTheme ? QColor("#2d2d30") : QColor("#f6f6f6");
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = this->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    
    this->setExtraSelections(extraSelections);
    lineNumberArea->update();
}

void CodeEditor::onUpdateRequest(const QRect &rect, int dy)
{
    if (dy != 0) {
        lineNumberArea->scroll(0, dy);
    } else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }
    
    if (rect.contains(this->viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void CodeEditor::updateLineNumberArea()
{
    lineNumberArea->update();
}

CppHighlighter::CppHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    keywordFormat.setForeground(QColor("#ff79c6"));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b" << "\\bdouble\\b" << "\\benum\\b"
                    << "\\bexplicit\\b" << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b" << "\\blong\\b"
                    << "\\bnamespace\\b" << "\\boperator\\b" << "\\bprivate\\b" << "\\bprotected\\b"
                    << "\\bpublic\\b" << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b" << "\\bslots\\b"
                    << "\\bstatic\\b" << "\\bstruct\\b" << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b" << "\\bvoid\\b" << "\\bvolatile\\b"
                    << "\\bbool\\b" << "\\breturn\\b" << "\\bif\\b" << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b"
                    << "\\bdo\\b" << "\\bswitch\\b" << "\\bcase\\b" << "\\bbreak\\b" << "\\bcontinue\\b"
                    << "\\bgoto\\b" << "\\bdefault\\b" << "\\btry\\b" << "\\bcatch\\b" << "\\bthrow\\b"
                    << "\\bnew\\b" << "\\bdelete\\b" << "\\bthis\\b" << "\\btrue\\b" << "\\bfalse\\b" << "\\bnullptr\\b";
    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
    classFormat.setForeground(QColor("#8be9fd"));
    classFormat.setFontWeight(QFont::Bold);
    HighlightingRule classRule;
    classRule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    classRule.format = classFormat;
    highlightingRules.append(classRule);
    singleLineCommentFormat.setForeground(QColor("#6272a4"));
    HighlightingRule singleLineCommentRule;
    singleLineCommentRule.pattern = QRegularExpression("//[^\n]*");
    singleLineCommentRule.format = singleLineCommentFormat;
    highlightingRules.append(singleLineCommentRule);
    multiLineCommentFormat.setForeground(QColor("#6272a4"));
    quotationFormat.setForeground(QColor("#f1fa8c"));
    HighlightingRule quotationRule;
    quotationRule.pattern = QRegularExpression("\".*\"");
    quotationRule.format = quotationFormat;
    highlightingRules.append(quotationRule);
    functionFormat.setForeground(QColor("#50fa7b"));
    HighlightingRule functionRule;
    functionRule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    functionRule.format = functionFormat;
    highlightingRules.append(functionRule);
    numberFormat.setForeground(QColor("#bd93f9"));
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b[0-9]+\\b");
    numberRule.format = numberFormat;
    highlightingRules.append(numberRule);
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void CppHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);
    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), isDarkTheme(true)
{
    settings = new QSettings("NOVA Editor", "NOVA Editor", this);
    translator = new QTranslator(this);
    
    setupUI();
    setupToolbar();
    loadLanguage();
    loadSession();
}

MainWindow::~MainWindow()
{
    saveSession();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSession();
    event->accept();
}

void MainWindow::setupUI()
{
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);
    setWindowTitle("NOVA Editor");
    resize(1200, 800);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::currentTabChanged);
    
    menuBar()->setVisible(false);
    
    setupSettingsTab();
}

void MainWindow::setupToolbar()
{
    mainToolBar = addToolBar("Main Toolbar");
    mainToolBar->setMovable(false);
    
    newAct = new QAction("New", this);
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    
    openAct = new QAction("Open", this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    
    saveAct = new QAction("Save", this);
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
    
    saveAsAct = new QAction("Save As", this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsFile);
    
    mainToolBar->addAction(newAct);
    mainToolBar->addAction(openAct);
    mainToolBar->addAction(saveAct);
    mainToolBar->addAction(saveAsAct);
}

void MainWindow::setupSettingsTab()
{
    QWidget *settingsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(settingsWidget);
    
    QGroupBox *languageGroup = new QGroupBox("Language");
    QHBoxLayout *languageLayout = new QHBoxLayout(languageGroup);
    languageLayout->addWidget(new QLabel("Interface Language:"));
    languageCombo = new QComboBox();
    languageCombo->addItem("English", "en");
    languageCombo->addItem("Русский", "ru");
    languageLayout->addWidget(languageCombo);
    layout->addWidget(languageGroup);
    
    QGroupBox *themeGroup = new QGroupBox("Theme");
    QHBoxLayout *themeLayout = new QHBoxLayout(themeGroup);
    themeLayout->addWidget(new QLabel("Color Theme:"));
    themeCombo = new QComboBox();
    themeCombo->addItem("Dark", "dark");
    themeCombo->addItem("Light", "light");
    themeLayout->addWidget(themeCombo);
    layout->addWidget(themeGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(settingsWidget, "Settings");
    
    QString savedLanguage = settings->value("language", "en").toString();
    int langIndex = languageCombo->findData(savedLanguage);
    if (langIndex >= 0) languageCombo->setCurrentIndex(langIndex);
    
    QString savedTheme = settings->value("theme", "dark").toString();
    int themeIndex = themeCombo->findData(savedTheme);
    if (themeIndex >= 0) themeCombo->setCurrentIndex(themeIndex);
    
    connect(languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changeLanguage);
    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changeTheme);
    
    applyTheme(savedTheme == "dark");
}

void MainWindow::applyTheme(bool dark)
{
    isDarkTheme = dark;
    
    if (dark) {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(darkPalette);
        setStyleSheet("QTabWidget::pane { border: 1px solid #444; background-color: #2b2b2b; }"
                     "QTabBar::tab { background-color: #353535; color: white; padding: 8px; }"
                     "QTabBar::tab:selected { background-color: #2b2b2b; }"
                     "QPlainTextEdit { background-color: #1e1e1e; color: #f8f8f2; border: none; }"
                     "QGroupBox { color: white; }"
                     "QToolBar { background-color: #353535; border: none; }");
    } else {
        qApp->setPalette(style()->standardPalette());
        setStyleSheet("QTabWidget::pane { border: 1px solid #ccc; }"
                     "QTabBar::tab { background-color: #f0f0f0; color: black; padding: 8px; }"
                     "QTabBar::tab:selected { background-color: white; }"
                     "QPlainTextEdit { background-color: white; color: black; border: none; }"
                     "QToolBar { background-color: #f0f0f0; border: none; }");
    }
    
    for (int i = 1; i < tabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (editor) {
            editor->setIsDarkTheme(dark);
            editor->highlightCurrentLine();
            editor->getLineNumberArea()->update();
        }
    }
}

void MainWindow::loadLanguage()
{
    QString lang = settings->value("language", "en").toString();
    
    if (lang == "ru") {
        newAct->setText("Новый");
        openAct->setText("Открыть");
        saveAct->setText("Сохранить");
        saveAsAct->setText("Сохранить как");
        
        if (tabWidget->count() > 0) {
            tabWidget->setTabText(0, "Настройки");
        }
        
        languageCombo->setItemText(0, "English");
        languageCombo->setItemText(1, "Русский");
        themeCombo->setItemText(0, "Тёмная");
        themeCombo->setItemText(1, "Светлая");
        
        QList<QGroupBox*> groups = findChildren<QGroupBox*>();
        for (QGroupBox *group : groups) {
            if (group->title() == "Language") group->setTitle("Язык");
            if (group->title() == "Theme") group->setTitle("Тема");
        }
        
        QList<QLabel*> labels = findChildren<QLabel*>();
        for (QLabel *label : labels) {
            if (label->text() == "Interface Language:") label->setText("Язык интерфейса:");
            if (label->text() == "Color Theme:") label->setText("Цветовая тема:");
        }
    } else {
        newAct->setText("New");
        openAct->setText("Open");
        saveAct->setText("Save");
        saveAsAct->setText("Save As");
        
        if (tabWidget->count() > 0) {
            tabWidget->setTabText(0, "Settings");
        }
        
        languageCombo->setItemText(0, "English");
        languageCombo->setItemText(1, "Русский");
        themeCombo->setItemText(0, "Dark");
        themeCombo->setItemText(1, "Light");
        
        QList<QGroupBox*> groups = findChildren<QGroupBox*>();
        for (QGroupBox *group : groups) {
            if (group->title() == "Язык") group->setTitle("Language");
            if (group->title() == "Тема") group->setTitle("Theme");
        }
        
        QList<QLabel*> labels = findChildren<QLabel*>();
        for (QLabel *label : labels) {
            if (label->text() == "Язык интерфейса:") label->setText("Interface Language:");
            if (label->text() == "Цветовая тема:") label->setText("Color Theme:");
        }
    }
}

void MainWindow::changeLanguage(int index)
{
    QString lang = languageCombo->itemData(index).toString();
    settings->setValue("language", lang);
    loadLanguage();
}

void MainWindow::changeTheme(int index)
{
    QString theme = themeCombo->itemData(index).toString();
    settings->setValue("theme", theme);
    applyTheme(theme == "dark");
}

void MainWindow::loadSession()
{
    int tabCount = settings->beginReadArray("tabs");
    for (int i = 0; i < tabCount; ++i) {
        settings->setArrayIndex(i);
        QString filePath = settings->value("filePath").toString();
        QString content = settings->value("content").toString();
        
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                CodeEditor *editor = createEditor();
                editor->setPlainText(QTextStream(&file).readAll());
                tabWidget->addTab(editor, QFileInfo(filePath).fileName());
                file.close();
            }
        } else if (!content.isEmpty()) {
            CodeEditor *editor = createEditor();
            editor->setPlainText(content);
            tabWidget->addTab(editor, "Untitled");
        }
    }
    settings->endArray();
    
    if (tabWidget->count() == 0) {
        newFile();
    }
}

void MainWindow::saveSession()
{
    settings->beginWriteArray("tabs");
    int saveIndex = 0;
    
    for (int i = 1; i < tabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (editor) {
            settings->setArrayIndex(saveIndex++);
            QString filePath = "";
            if (!currentFile.isEmpty() && tabWidget->currentIndex() == i) {
                filePath = currentFile;
            }
            settings->setValue("filePath", filePath);
            settings->setValue("content", editor->toPlainText());
        }
    }
    settings->endArray();
}

CodeEditor* MainWindow::createEditor()
{
    CodeEditor *editor = new CodeEditor();
    QFont font("Monospace");
    font.setPointSize(12);
    editor->setFont(font);
    new CppHighlighter(editor->document());
    
    editor->setIsDarkTheme(isDarkTheme);
    
    connect(editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::documentModified);
    
    return editor;
}

CodeEditor* MainWindow::currentEditor()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex > 0) {
        return qobject_cast<CodeEditor*>(tabWidget->widget(currentIndex));
    }
    return nullptr;
}

void MainWindow::newFile()
{
    CodeEditor *editor = createEditor();
    int index = tabWidget->addTab(editor, "Untitled");
    tabWidget->setCurrentIndex(index);
    updateTitle();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            CodeEditor *editor = createEditor();
            editor->setPlainText(QTextStream(&file).readAll());
            int index = tabWidget->addTab(editor, QFileInfo(fileName).fileName());
            tabWidget->setCurrentIndex(index);
            setCurrentFile(fileName);
            file.close();
        }
    }
}

void MainWindow::saveFile()
{
    CodeEditor *editor = currentEditor();
    if (!editor) return;
    
    if (currentFile.isEmpty()) {
        saveAsFile();
    } else {
        QFile file(currentFile);
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream out(&file);
            out << editor->toPlainText();
            file.close();
            editor->document()->setModified(false);
            tabWidget->setTabText(tabWidget->currentIndex(), QFileInfo(currentFile).fileName());
            statusBar()->showMessage("File saved", 2000);
        }
    }
}

void MainWindow::saveAsFile()
{
    CodeEditor *editor = currentEditor();
    if (!editor) return;
    
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream out(&file);
            out << editor->toPlainText();
            file.close();
            setCurrentFile(fileName);
            tabWidget->setTabText(tabWidget->currentIndex(), QFileInfo(fileName).fileName());
            editor->document()->setModified(false);
            statusBar()->showMessage("File saved", 2000);
        }
    }
}

void MainWindow::closeTab(int index)
{
    if (index == 0) return;
    
    QWidget *widget = tabWidget->widget(index);
    if (widget) {
        tabWidget->removeTab(index);
        widget->deleteLater();
    }
    updateTitle();
}

void MainWindow::currentTabChanged(int index)
{
    if (index > 0) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(index));
        if (editor) {
            QString tabText = tabWidget->tabText(index);
            if (tabText.endsWith("*")) {
                currentFile = "";
            } else {
                for (int i = 1; i < tabWidget->count(); ++i) {
                    if (i == index) {
                        currentFile = tabText;
                        break;
                    }
                }
            }
        }
    }
    updateTitle();
}

void MainWindow::documentModified()
{
    CodeEditor *editor = qobject_cast<CodeEditor*>(sender()->parent());
    if (editor) {
        for (int i = 1; i < tabWidget->count(); ++i) {
            if (tabWidget->widget(i) == editor) {
                QString tabText = tabWidget->tabText(i);
                if (editor->document()->isModified()) {
                    if (!tabText.endsWith("*")) {
                        tabWidget->setTabText(i, tabText + "*");
                    }
                } else {
                    if (tabText.endsWith("*")) {
                        tabWidget->setTabText(i, tabText.left(tabText.length() - 1));
                    }
                }
                break;
            }
        }
    }
}

void MainWindow::updateTitle()
{
    if (tabWidget->count() > 1 && tabWidget->currentIndex() > 0) {
        QString title = tabWidget->tabText(tabWidget->currentIndex());
        if (title.endsWith("*")) {
            title = title.left(title.length() - 1) + " - NOVA Editor";
        } else {
            title = title + " - NOVA Editor";
        }
        setWindowTitle(title);
    } else {
        setWindowTitle("NOVA Editor");
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
}