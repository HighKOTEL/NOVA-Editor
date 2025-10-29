#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSettings>
#include <QComboBox>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTranslator>
#include <QCloseEvent>
#include <QWidget>
#include <QScrollBar>
#include <QPainter>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = nullptr);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void updateLineNumberArea();
    LineNumberArea* getLineNumberArea() { return lineNumberArea; }
    bool getIsDarkTheme() const { return isDarkTheme; }
    void setIsDarkTheme(bool dark) { isDarkTheme = dark; }
    void highlightCurrentLine();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void onUpdateRequest(const QRect &rect, int dy);

private:
    LineNumberArea *lineNumberArea;
    bool isDarkTheme;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }
private:
    CodeEditor *codeEditor;
};

class CppHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    CppHighlighter(QTextDocument *parent = nullptr);
protected:
    void highlightBlock(const QString &text) override;
private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat numberFormat;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
protected:
    void closeEvent(QCloseEvent *event) override;
    
private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void closeTab(int index);
    void currentTabChanged(int index);
    void changeLanguage(int index);
    void changeTheme(int index);
    void updateTitle();
    void documentModified();
    
private:
    void setupUI();
    void setupToolbar();
    void setupSettingsTab();
    void applyTheme(bool dark);
    void loadLanguage();
    void loadSession();
    void saveSession();
    void setCurrentFile(const QString &fileName);
    CodeEditor* currentEditor();
    CodeEditor* createEditor();
    void retranslateUI();
    
    QTabWidget *tabWidget;
    QToolBar *mainToolBar;
    QSettings *settings;
    QTranslator *translator;
    
    QComboBox *languageCombo;
    QComboBox *themeCombo;
    
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    
    QString currentFile;
    bool isDarkTheme;
};

#endif