#include <iostream>
#include <string>
#include "window.hpp"
#include "json/json.hpp"

using json = nlohmann::json;

Window::Window()
{
    QWidget *MainWidget = new QWidget;         // Main Widget
    QVBoxLayout *MainLayout = new QVBoxLayout; // Vertical Layout
    createMenu();                              // Call createMenu() to configure MenuBar
    setMenuBar(menuBar);                       // set MenuBar
    MainLayout->addWidget(createComboBox());   // set ComboBox
    MainLayout->addWidget(createTextBox());    // set TextBox
    MainWidget->setLayout(MainLayout);         // set Layout
    setCentralWidget(MainWidget);              // set Main Widget
    setWindowTitle("Quran app");               // set Window Title
    setMinimumWidth(800);                      // set Minimum Width of Window
    setMinimumHeight(600);                     // set Minimum Height of Window
    connect(surah, SIGNAL(currentTextChanged(QString)), this, SLOT(showSurah())); // Watch Changes in ComboBox surah
    connect(translation, SIGNAL(currentTextChanged(QString)), this, SLOT(showTranslation())); // Watch Changes in ComboBox translation
}

void Window::createMenu()
{
    // Function to Configure MenuBar

    menuBar = new QMenuBar;
    Menu = new QMenu(tr("Menu"), this);
    darkmode = Menu->addAction(tr("Dark Mode")); // Add Dark Mode Menu Entry
    about = Menu->addAction(tr("About"));        // Add About Menu Entry
    darkmode->setCheckable(true);
    menuBar->addMenu(Menu);
    connect(about, SIGNAL(triggered()), this, SLOT(showAbout())); 
    connect(darkmode, SIGNAL(triggered()), this, SLOT(setDarkMode()));
}

QGroupBox *Window::createComboBox()
{
    // Function to Configure ComboBox

    std::vector<std::string>data = getMetadata();
    QGroupBox *group = new QGroupBox;      // GroupBox ( To Group Widgets )
    QHBoxLayout *layout = new QHBoxLayout; // Horizontal Layout
    surah = new QComboBox;      
    translation = new QComboBox;
    if(data.empty())
    {
        surah->addItem("No Internet"); // Add text to ComboBox
    }
    else
    {
        for(int i=0;i<114;i++)
        {
            surah->addItem(QString::number(i+1) + ". " + QString::fromStdString(data.at(i)));
        }
    }
    translation->addItem("English");
    translation->addItem("Hindi");
    translation->addItem("Malayalam");
    translation->addItem("Turkish");
    translation->addItem("Tamil");
    translation->addItem("Urdu");
    layout->addWidget(translation);
    layout->addWidget(surah);
    group->setLayout(layout);
    return group;
}

QGroupBox *Window::createTextBox()
{
    // Function to configure Text Box

    std::string surah_url = "https://api.alquran.cloud/v1/surah/";
    std::string translation_url = "https://api.alquran.cloud/v1/surah/";
    surah_url.append(std::to_string(surah_number) + "/");
    surah_url.append("quran-simple-enhanced");
    translation_url.append(std::to_string(surah_number) + "/");
    translation_url.append(edition);
    QGroupBox *group = new QGroupBox;
    QHBoxLayout *layout = new QHBoxLayout;
    show_surah = new QTextEdit;
    show_translation = new QTextEdit;
    getSurah(surah_url);
    getTranslation(translation_url);
    layout->addWidget(show_translation);
    layout->addWidget(show_surah);
    group->setLayout(layout);
    return group;
}

std::vector<std::string> Window::getMetadata()
{
    // Function to retrieve the surah Names

    std::vector<std::string>data;
    this->url = "https://api.alquran.cloud/v1/meta";
    try {
        json metadata = json::parse(this->curl_process()); // Parse the JSON data
        for(int i=0;i<114;i++)
        {
            data.push_back(metadata["data"]["surahs"]["references"][i]["englishName"]); 
        }
    } catch(nlohmann::json::type_error &err) {

        // caught if any errors encountered during JSON parsing

        QMessageBox::critical(this, "Error", "No Internet Connection , Please enable internet and restart the Application");
    }
    return data;
}

void Window::getSurah(std::string surah_url)
{
    
    this->url = surah_url;
    QVector<QString>parsed_data;
    try {
        json parse_surah = json::parse(curl_process());
        int ayahs = parse_surah["data"]["numberOfAyahs"];
        int count = 1;
        show_surah->setText(QString::fromStdString(parse_surah["data"]["name"].get<std::string>()));
        QTextCursor cursor = show_surah->textCursor();
        QTextBlockFormat textBlockFormat = cursor.blockFormat();
        textBlockFormat.setAlignment(Qt::AlignCenter);              // Align Text to Center
        cursor.mergeBlockFormat(textBlockFormat);
        for(int i=0;i<ayahs;i++)
        {
            parsed_data.push_back(QString::fromStdString(parse_surah["data"]["ayahs"][i]["text"].get<std::string>()));
        }
        for(QString ayah : parsed_data)
        {
            QString data =  QString::fromStdString(std::to_string(count)) + ". " + ayah;
            show_surah->append(data + "\n");
            count++;
        }
        show_surah->selectAll();
        show_surah->setFontPointSize(16); // set Font Size
        show_surah->setTextCursor(cursor);
        show_surah->setReadOnly(true);    // set Text Box Read Only
    } catch(nlohmann::json::type_error &err) {
        show_surah->setText(" Please Enable Internet to use this application ");
    }
}

void Window::getTranslation(std::string translation_url)
{
    this->url = translation_url;
    QVector<QString>parsed_data;
    try {

        json parse_surah = json::parse(curl_process());
        int ayahs = parse_surah["data"]["numberOfAyahs"];
        int count = 1;
        show_translation->setText(QString::fromStdString(parse_surah["data"]["englishName"].get<std::string>()));
        QTextCursor cursor = show_translation->textCursor();
        QTextBlockFormat textBlockFormat = cursor.blockFormat();
        textBlockFormat.setAlignment(Qt::AlignCenter);
        cursor.mergeBlockFormat(textBlockFormat);
        for(int i=0;i<ayahs;i++)
        {
            parsed_data.push_back(QString::fromStdString(parse_surah["data"]["ayahs"][i]["text"].get<std::string>()));
        }
        for(QString ayah : parsed_data)
        {
            QString data =  QString::fromStdString(std::to_string(count)) + ". " + ayah;
            show_translation->append(data + "\n");
            count++;
        }
        show_translation->selectAll();
        show_translation->setFontPointSize(16);
        show_translation->setTextCursor(cursor);
        show_translation->setReadOnly(true);
    } catch(nlohmann::json::type_error &err) {
        show_translation->setText(" Please Enable Internet to use this application ");
    }
}

void Window::showSurah()
{
    std::string surah_url = "https://api.alquran.cloud/v1/surah/";
    std::string translation_url = surah_url;
    std::vector<std::string>data;
    char *token =  strtok((char *)surah->currentText().toStdString().c_str(), "."); // Tokenize String
    while(token != NULL)
    {   
        // Strings before and after '.' character is pushed to vector

        data.push_back(token);
        token = strtok(NULL, ".");
    }
    surah_number = std::stoi(data.at(0));
    surah_url.append(std::to_string(surah_number) + "/");
    surah_url.append("quran-simple-enhanced");
    edition = getEdition(translation->currentText().toStdString());
    translation_url.append(std::to_string(surah_number) + "/");
    translation_url.append(edition);
    getSurah(surah_url);
    getTranslation(translation_url);
}

void Window::showTranslation()
{
    std::string translation_url = "https://api.alquran.cloud/v1/surah/";
    edition = getEdition(translation->currentText().toStdString());
    translation_url.append(std::to_string(surah_number) + "/");
    translation_url.append(edition);
    getTranslation(translation_url);
}

std::string Window::getEdition(std::string identifier)
{
    if(identifier == "English")
    {
        return "en.sahih";
    }
    else if(identifier == "Hindi")
    {
        return "hi.hindi";
    }
    else if(identifier == "Malayalam")
    {
        return "ml.abdulhameed";
    }
    else if(identifier == "Tamil")
    {
        return "ta.tamil";
    }
    else if(identifier == "Turkish")
    {
        return "tr.ates";
    }
    else if(identifier == "Urdu")
    {
        return "ur.ahmedali";
    }
}

void Window::showAbout()
{
    QWidget *AboutWindow = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    QPixmap pixmap("/opt/qapp/qapp-198x198.png");
    QFont hfont("Arial", 15, QFont::Bold);
    QFont nfont("Arial", 13, QFont::AnyStyle);
    QFont ffont("Arial", 12, QFont::AnyStyle);
    ffont.setItalic(true);
    QLabel *icon = new QLabel;
    QLabel *header = new QLabel("A Project by Muslim Programmers Community");
    QLabel *Discord = new QLabel("Discord : discord.gg/7cnWVc8qgb");
    QLabel *Instagram = new QLabel("Instagram : @muslimpgmrs");
    QLabel *footer = new QLabel("Emerging since 2021");
    icon->setPixmap(pixmap);
    icon->setAlignment(Qt::AlignCenter);
    icon->setGeometry(QRect(312, 454, 21, 20));
    header->setAlignment(Qt::AlignCenter);
    header->setFont(hfont);
    header->setGeometry(QRect(312, 454, 21, 20));
    Discord->setFont(nfont);
    Discord->setGeometry(QRect(300, 400, 50, 50));
    Instagram->setFont(nfont);
    Discord->setTextInteractionFlags(Qt::TextSelectableByMouse);
    Discord->setAlignment(Qt::AlignCenter);
    Instagram->setAlignment(Qt::AlignCenter);
    Instagram->setTextInteractionFlags(Qt::TextSelectableByMouse);
    footer->setFont(ffont);
    footer->setAlignment(Qt::AlignCenter);
    layout->addWidget(icon);
    layout->addWidget(header);
    layout->addWidget(Discord);
    layout->addWidget(Instagram);
    layout->addWidget(footer);
    AboutWindow->setLayout(layout);
    AboutWindow->setMinimumSize(602,443);
    AboutWindow->setStyleSheet("background-color: #333333; color: white;");
    AboutWindow->show();
}

void Window::setDarkMode()
{
    if(dark_mode_enabled)
    {
        setStyleSheet("");
        dark_mode_enabled = false;
    }
    else
    {
        setStyleSheet("*{background-color: #333333; color: white;} QMenuBar:item:hover {background-color: blue;}");
        dark_mode_enabled = true;
    }
}
