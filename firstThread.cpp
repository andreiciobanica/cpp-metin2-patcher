#include "firstThread.h"
#include "config.h"

firstThread::firstThread(QObject *parent)
	: QThread(parent)
{
}

firstThread::firstThread()
{

}

int create_folders(std::vector<string>folders) {
    for (auto folder : folders) {
        //ui->nameLabel->setText(QString::fromStdString("Creating: " + folder));
        fs::create_directories(folder.c_str());
    }
    return 0;
}

std::vector<std::string> explode(std::string const& s, char delim) {
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); )
    {
        result.push_back(std::move(token));
    }

    return result;
}

std::string split_from_last(std::string const& s, char delim) {
    return s.substr(s.find_last_of(delim) + 1, s.length());
}

std::string delete_extension(std::string const& s) {
    return s.substr(0, s.find_first_of('.'));
}

std::string get_extension(std::string const& s) {
    return s.substr(s.find_first_of('.') + 1, s.length());
}

std::string get_folders(std::string const& s) {
    return s.substr(0, s.find_last_of('/') + 1);
}

void replace_all_in_path(std::string& source, string const& find, string const& replace)
{
    for (string::size_type i = 0; (i = source.find(find, i)) != string::npos;)
    {
        source.replace(i, find.length(), replace);
        i += replace.length();
    }
}

inline std::string
SHA256(const char* const path)
{
    std::ifstream fp(path, std::ios::in | std::ios::binary);

    constexpr const std::size_t buffer_size{ 1 << 12 };
    char buffer[buffer_size];

    unsigned char hash[SHA256_DIGEST_LENGTH] = { 0 };

    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    while (fp.good()) {
        fp.read(buffer, buffer_size);
        SHA256_Update(&ctx, buffer, fp.gcount());
    }

    SHA256_Final(hash, &ctx);
    fp.close();

    std::ostringstream os;
    os << std::hex << std::setfill('0');

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        os << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }

    return os.str();
}

void firstThread::run() {
    QStringList arguments;

    std::string server = PatcherConfig::ServerUrl;

    std::string absolutepath = fs::current_path().string();
    absolutepath = absolutepath + "\\";

    //std::map<std::pair<string, std::pair<string, string>>, string> listaPC;
    //std::string fileName;
    //std::string fileExtension;
    //std::vector<string> foldersPC;
    /*for (const auto& dirEntry : fs::recursive_directory_iterator(fs::current_path())) {
        if (!fs::is_directory(dirEntry.path())) {
            string filePath = dirEntry.path().string();
            filePath.erase(filePath.begin(), filePath.begin() + absolutepath.length());
            //qDebug("%s", filePath.c_str());
            string fileHash = SHA256(filePath.c_str());
            fileName = split_from_last(filePath, '\\');
            fileExtension = get_extension(fileName);
            fileName = delete_extension(fileName);
            replace_all_in_path(filePath, "\\", "/");
            listaPC.insert({ { filePath, {fileName, fileExtension} }, fileHash });
        }
        else {
            string folderPath = dirEntry.path().string();
            folderPath.erase(folderPath.begin(), folderPath.begin() + absolutepath.length());
            foldersPC.push_back(folderPath);
        }
    }*/

    std::map<std::pair<string, std::pair<string, string>>, string> patchList;
    std::vector<string> folderList;
    std::map<string, string> exceptionList;
    std::ifstream file;

    // load the hash list downloaded from the server
    file.open("patchlist.txt");
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string filePath, fileName, fileExtension, hash;
        if (!(iss >> filePath >> hash)) { break; }
        fileName = split_from_last(filePath, '/');
        fileExtension = get_extension(fileName);
        fileName = delete_extension(fileName);
        patchList.insert({ { filePath, {fileName, fileExtension} }, hash });
    }
    file.close();

    // load the folder list
    file.open("folders.txt");
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string folder;
        if (!(iss >> folder)) { break; }
        folderList.push_back(folder);
    }
    file.close();
    create_folders(folderList);

    // load the exception list
    file.open("exceptions.txt");
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string folder;
        std::string extensionLine;
        if (!(iss >> folder >> extensionLine)) { break; }
        exceptionList.insert({ folder, extensionLine });
    }
    file.close();

    for (auto filePath : patchList) {
        std::string folderPath = get_folders(filePath.first.first);
        std::string fileName = filePath.first.second.first;
        std::string fileExtension = filePath.first.second.second;
        std::string fileDownload = server + folderPath + fileName + '.' + fileExtension;
        // check if the file from the server exists on the PC
        if (!fs::exists(folderPath + fileName + '.' + fileExtension)) {
            arguments.append(QString::fromStdString(fileDownload));
        }
        else {
            // the file exists, but check it against the exception list
            if (!(exceptionList.find(folderPath) == exceptionList.end())) {
                std::vector<string>exceptionExtension = explode(exceptionList.find(folderPath)->second, '|');
                if (std::find(exceptionExtension.begin(), exceptionExtension.end(), fileExtension) != exceptionExtension.end()) {

                }
                else {
                    arguments.append(QString::fromStdString(fileDownload));
                }
            }
            // the file exists, so check its integrity by comparing the hashes
            else if (!(SHA256((folderPath + fileName + '.' + fileExtension).c_str()) == patchList.at(filePath.first))) {
                //qDebug("%s", fileDownload.c_str());
                arguments.append(QString::fromStdString(fileDownload));
            }
        }
    }
    //emit updateTotalProgressMaximumValue((int)arguments.size());
    emit afterFirstThread(arguments);
}
