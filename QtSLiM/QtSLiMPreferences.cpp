#include "QtSLiMPreferences.h"
#include "ui_QtSLiMPreferences.h"

#include <QSettings>


//
//  QSettings keys for the prefs we control; these are private
//

static const char *QtSLiMAppStartupAction = "QtSLiMAppStartupAction";
static const char *QtSLiMDisplayFontFamily = "QtSLiMDisplayFontFamily";
static const char *QtSLiMDisplayFontSize = "QtSLiMDisplayFontSize";
static const char *QtSLiMSyntaxHighlightScript = "QtSLiMSyntaxHighlightScript";
static const char *QtSLiMSyntaxHighlightOutput = "QtSLiMSyntaxHighlightOutput";


static QFont &defaultDisplayFont(void)
{
    // This function determines the default font chosen when the user has expressed no preference.
    // It depends upon font availability, so it can't be hard-coded.
    static QFont *defaultFont = nullptr;
    
    if (!defaultFont)
    {
        QFontDatabase fontdb;
        QStringList families = fontdb.families();
        
        // Use filter() to look for matches, since the foundry can be appended after the name (why isn't this easier??)
        if (families.filter("Courier New").size() > 0)              // good on Mac
            defaultFont = new QFont("Courier New", 13);
        else if (families.filter("Menlo").size() > 0)               // good on Mac
            defaultFont = new QFont("Menlo", 12);
        else if (families.filter("Ubuntu Mono").size() > 0)         // good on Ubuntu
            defaultFont = new QFont("Ubuntu Mono", 11);
        else if (families.filter("DejaVu Sans Mono").size() > 0)    // good on Ubuntu
            defaultFont = new QFont("DejaVu Sans Mono", 9);
        else
            defaultFont = new QFont("Courier", 10);                 // a reasonable default that should be omnipresent
    }
    
    return *defaultFont;
}


//
//  QtSLiMPreferencesNotifier: the pref supplier and notifier
//

QtSLiMPreferencesNotifier &QtSLiMPreferencesNotifier::instance(void)
{
    static QtSLiMPreferencesNotifier inst;
    
    return inst;
}

// pref value fetching

int QtSLiMPreferencesNotifier::appStartupPref(void)
{
    QSettings settings;
    
    return settings.value(QtSLiMAppStartupAction, QVariant(1)).toInt();
}

QFont QtSLiMPreferencesNotifier::displayFontPref(int *tabWidth)
{
    QFont &defaultFont = defaultDisplayFont();
    QString defaultFamily = defaultFont.family();
    int defaultSize = defaultFont.pointSize();
    
    QSettings settings;
    QString fontFamily = settings.value(QtSLiMDisplayFontFamily, QVariant(defaultFamily)).toString();
    int fontSize = settings.value(QtSLiMDisplayFontSize, QVariant(defaultSize)).toInt();
    QFont font(fontFamily, fontSize);
    
    font.setFixedPitch(true);    // I think this is a hint to help QFont match to similar fonts?
    
    if (tabWidth)
    {
        QFontMetrics fm(font);
        
        //*tabWidth = fm.horizontalAdvance("   ");   // added in Qt 5.11
        *tabWidth = fm.width("   ");                 // deprecated (in 5.11, I assume)
    }
    
    return font;
}

bool QtSLiMPreferencesNotifier::scriptSyntaxHighlightPref(void)
{
    QSettings settings;
    
    return settings.value(QtSLiMSyntaxHighlightScript, QVariant(true)).toBool();
}

bool QtSLiMPreferencesNotifier::outputSyntaxHighlightPref(void)
{
    QSettings settings;
    
    return settings.value(QtSLiMSyntaxHighlightOutput, QVariant(true)).toBool();
}

// slots; these update the settings and then emit new signals

void QtSLiMPreferencesNotifier::startupRadioChanged()
{
    QtSLiMPreferences &prefsUI = QtSLiMPreferences::instance();
    QSettings settings;
    
    if (prefsUI.ui->startupRadioOpenFile->isChecked())
        settings.setValue(QtSLiMAppStartupAction, QVariant(1));
    else if (prefsUI.ui->startupRadioCreateNew->isChecked())
        settings.setValue(QtSLiMAppStartupAction, QVariant(2));
    
    emit appStartupPrefChanged();
}

void QtSLiMPreferencesNotifier::fontChanged(const QFont &newFont)
{
    QString fontFamily = newFont.family();
    QSettings settings;
    
    settings.setValue(QtSLiMDisplayFontFamily, QVariant(fontFamily));
    
    emit displayFontPrefChanged();
}

void QtSLiMPreferencesNotifier::fontSizeChanged(int newSize)
{
    QSettings settings;
    
    settings.setValue(QtSLiMDisplayFontSize, QVariant(newSize));
    
    emit displayFontPrefChanged();
}

void QtSLiMPreferencesNotifier::syntaxHighlightScriptToggled()
{
    QtSLiMPreferences &prefsUI = QtSLiMPreferences::instance();
    QSettings settings;
    
    settings.setValue(QtSLiMSyntaxHighlightScript, QVariant(prefsUI.ui->syntaxHighlightScript->isChecked()));
    
    emit scriptSyntaxHighlightPrefChanged();
}

void QtSLiMPreferencesNotifier::syntaxHighlightOutputToggled()
{
    QtSLiMPreferences &prefsUI = QtSLiMPreferences::instance();
    QSettings settings;
    
    settings.setValue(QtSLiMSyntaxHighlightOutput, QVariant(prefsUI.ui->syntaxHighlightOutput->isChecked()));
    
    emit outputSyntaxHighlightPrefChanged();
}

void QtSLiMPreferencesNotifier::resetSuppressedClicked()
{
    // All "do not show this again" settings should be removed here
    // There is no signal rebroadcast for this; nobody should cache these flags
    QSettings settings;
    settings.remove("QtSLiMSuppressScriptCheckSuccessPanel");
}


//
//  QtSLiMPreferences: the actual UI class
//

QtSLiMPreferences &QtSLiMPreferences::instance(void)
{
    static QtSLiMPreferences inst(nullptr);
    
    return inst;
}

QtSLiMPreferences::QtSLiMPreferences(QWidget *parent) : QDialog(parent), ui(new Ui::QtSLiMPreferences)
{
    ui->setupUi(this);
    
    // set the initial state of the UI elements from QtSLiMPreferencesNotifier
    QtSLiMPreferencesNotifier &prefsNotifier = QtSLiMPreferencesNotifier::instance();
    
    ui->startupRadioCreateNew->setChecked(prefsNotifier.appStartupPref() == 1);
    ui->startupRadioOpenFile->setChecked(prefsNotifier.appStartupPref() == 2);
    
    ui->fontComboBox->setCurrentFont(prefsNotifier.displayFontPref());
    ui->fontSizeSpinBox->setValue(prefsNotifier.displayFontPref().pointSize());
    
    ui->syntaxHighlightScript->setChecked(prefsNotifier.scriptSyntaxHighlightPref());
    ui->syntaxHighlightOutput->setChecked(prefsNotifier.outputSyntaxHighlightPref());
    
    // connect the UI elements to QtSLiMPreferencesNotifier
    QtSLiMPreferencesNotifier *notifier = &QtSLiMPreferencesNotifier::instance();
    
    connect(ui->startupRadioOpenFile, &QRadioButton::toggled, notifier, &QtSLiMPreferencesNotifier::startupRadioChanged);
    connect(ui->startupRadioCreateNew, &QRadioButton::toggled, notifier, &QtSLiMPreferencesNotifier::startupRadioChanged);
    
    connect(ui->fontComboBox, &QFontComboBox::currentFontChanged, notifier, &QtSLiMPreferencesNotifier::fontChanged);
    connect(ui->fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), notifier, &QtSLiMPreferencesNotifier::fontSizeChanged);

    connect(ui->syntaxHighlightScript, &QCheckBox::toggled, notifier, &QtSLiMPreferencesNotifier::syntaxHighlightScriptToggled);
    connect(ui->syntaxHighlightOutput, &QCheckBox::toggled, notifier, &QtSLiMPreferencesNotifier::syntaxHighlightOutputToggled);

    connect(ui->resetSuppressedButton, &QPushButton::clicked, notifier, &QtSLiMPreferencesNotifier::resetSuppressedClicked);
}

QtSLiMPreferences::~QtSLiMPreferences()
{
    delete ui;
}



























