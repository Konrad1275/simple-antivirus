#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <openssl/aes.h>
#include <openssl/md5.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
#pragma ide diagnostic ignored "UnusedLocalVariable"
#define BUFF_SIZE 256

//using namespace std;

void encrypt(const char *path) {
  int bytes_read = 0;

  unsigned char indata[AES_BLOCK_SIZE];
  unsigned char outdata[AES_BLOCK_SIZE];

  unsigned char ckey[] = "thiskeyisverybad";
  unsigned char ivec[] = "dontusethisinput";

  AES_KEY key;

  AES_set_encrypt_key(ckey, 128, &key);

  int num = 0;

  FILE *ifp = fopen(path, "rb");
  FILE *ofp = fopen("tempOutput", "wb");

  while (true) {
    bytes_read = fread(indata, 1, AES_BLOCK_SIZE, ifp);

    AES_cfb128_encrypt(indata, outdata, bytes_read, &key, ivec, &num,
                       AES_ENCRYPT);

    fwrite(outdata, 1, bytes_read, ofp);
    if (bytes_read < AES_BLOCK_SIZE)
      break;
  }
  fclose(ifp);
  fclose(ofp);
    cout << path << endl;
  filesystem::rename("tempOutput", path);
}
void decrypt(const char *path) {
  int bytes_read = 0;

  unsigned char indata[AES_BLOCK_SIZE];
  unsigned char outdata[AES_BLOCK_SIZE];

  unsigned char ckey[] = "thiskeyisverybad";
  unsigned char ivec[] = "dontusethisinput";

  AES_KEY key;

  AES_set_encrypt_key(ckey, 128, &key);

  int num = 0;

  FILE *ifp = fopen(path, "rb");
  FILE *ofp = fopen("tempDecrypted", "wb");

  while (true) {
    bytes_read = fread(indata, 1, AES_BLOCK_SIZE, ifp);

    AES_cfb128_encrypt(indata, outdata, bytes_read, &key, ivec, &num,
                       AES_DECRYPT);

    fwrite(outdata, 1, bytes_read, ofp);
    if (bytes_read < AES_BLOCK_SIZE)
      break;
  }
  fclose(ifp);
  fclose(ofp);

  filesystem::rename("tempDecrypted", path);
}

string hashingMD5(const char *filePath) {
  int fd;
  char buff[BUFF_SIZE];
  int i;

  MD5_CTX md5_ctx;
  unsigned char md5_hash[MD5_DIGEST_LENGTH];

  MD5_Init(&md5_ctx);

  fd = open(filePath, O_RDONLY, 0);
  do {
    i = read(fd, buff, BUFF_SIZE);
    MD5_Update(&md5_ctx, buff, i);
  } while (i > 0);
  close(fd);
  MD5_Final(md5_hash, &md5_ctx);

  // converting hash to string
  char md5String[33];
  for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    sprintf(&md5String[i * 2], "%02x", (unsigned int)md5_hash[i]);
  return (string(md5String));
}

string CWD() {
  char *cwd = getcwd(0, 0);
  string working_directory(cwd);
  free(cwd);
  return working_directory;
}

void quarantine(string const & path, vector<string> &values) {
  cout << path << " jest poddawany kwarantannie" << endl;
  values.push_back(path);
  if (!filesystem::exists("quarantine"))
    filesystem::create_directories("quarantine");
  encrypt(path.c_str());
  //filesystem::rename(path, "quarantine/"+path.substr(path.find_last_of("/")+1));
  filesystem::rename(path, "/home/konri/Desktop/bso_antywirus/quarantine/quarantined"+to_string(values.size()));
}

void scaning(const char *name, vector<string> virus, int &scanned,
             int &detected, vector<string> &quar) {
  DIR *dir;
  struct dirent *entry;
  string current = "/";
  string auxiliary;
  char choice;
  if (strcmp(name, ".") != 0)
    current += string(name);
  if (!(dir = opendir(name))) {
    if (filesystem::exists(name)) {
      scanned++;
      auxiliary = CWD() + current;
      //cout<<auxiliary<<" "<<hashingMD5(auxiliary.c_str())<<endl;
      if (count(virus.begin(), virus.end(), hashingMD5(auxiliary.c_str()))) {
        detected++;
        cout << "\033[1;31mVIRUS!!! \033[0m";
        cout << auxiliary << endl;
        cout << "chcesz poddac ten plik kwarantannie? (Y/N)" << endl << ">";
        cin >> choice;
        if (choice == 'Y' || choice == 'y') {
          if (count(quar.begin(), quar.end(), auxiliary))
            cout << "plik juz jest w kwarantannie" << endl;
          else
            quarantine(auxiliary, quar);
        }
      }
      return;
    }
  } else {
    while ((entry = readdir(dir)) != NULL) {
      scanned++;
      if (entry->d_type == DT_DIR) {
        char path[1024];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
          continue;
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        scaning(path, virus, scanned, detected, quar);
      } else if (entry->d_type == DT_REG){ 
        if (current != "/") {
          auxiliary = CWD() + current.substr(2, current.size() - 2) + "/" + entry->d_name;
        } else {
          auxiliary = CWD() + current + entry->d_name;
        }
        //cout<<auxiliary<<" "<<hashingMD5(auxiliary.c_str())<<endl;
        if (count(virus.begin(), virus.end(), hashingMD5(auxiliary.c_str()))) {
          detected++;
          cout << "\033[1;31mVIRUS!!! \033[0m";
          cout << auxiliary << endl;
          cout << "chcesz poddac ten plik kwarantannie? (Y/N)" << endl << ">";
          cin >> choice;
          if (choice == 'Y' || choice == 'y') {
            if (count(quar.begin(), quar.end(), auxiliary))
              cout << "plik juz jest w kwarantannie" << endl;
            else {
              quarantine(auxiliary, quar);
            }
          }
        }
      }
    }
    closedir(dir);
  }
}

vector<string> loadHashes() {
  ifstream indata("VirusDatabase.dat");
  string hash;
  vector<string> result;
  while (getline(indata, hash)) {
    result.push_back(hash);
  }
  indata.close();
  return result;
}

vector<string> loadQuarantined() {
  ifstream indata("Quarantined.dat");
  string hash;
  vector<string> result;
  while (getline(indata, hash)) {
    result.push_back(hash);
  }
  indata.close();
  return result;
}

void saveQuarantined(const vector<string> & argument) {
  ofstream indata("Quarantined.dat");
  for (const string & x : argument) {
    indata << x << endl;
  }
  indata.close();
}

void generateHashes(const vector<string> & data) {
  for (const string & n : data) {
    cout << hashingMD5(n.c_str()) << " ";
    // cout<<hashing2(n.c_str())<<endl;
  }
}

auto unquarantine(int number, vector<string> &values) {
  filesystem::rename("/home/konri/Desktop/bso_antywirus/quarantine/quarantined"+to_string(number),values[number -1]);
  string value = values[number - 1];
  decrypt(value.c_str());
  values.erase(values.begin() + number);
  return value;
}

void interface() {
  cout << "\033[1;35mWitaj \033[0m" << endl;
  cout << "╔═══════════════════════════╗" << endl;
  cout << "║ wybierz opcje             ║" << endl;
  cout << "╠═══════════════════════════╣" << endl;
  cout << "║ S - skanowanie            ║" << endl;
  cout << "║ T - statystyki            ║" << endl;
  cout << "║ Q - pliki w kwarantannie  ║" << endl;
  cout << "║ A - dodaj plik do bazy    ║" << endl;
  cout << "║ E - wyjscie               ║" << endl;
  cout << "╚═══════════════════════════╝" << endl;
}

vector<string> fileListing(const char *name) {
  vector<string> result;
  DIR *dir;
  struct dirent *entry;
  string auxiliary;
  string current = "/";
  if (strcmp(name, ".") != 0)
    current += string(name);
  if (!(dir = opendir(name)))
    return result;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      char path[1024];
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
      if (current != "/") {
        // result.push_back(CWD()+current.substr(2,current.size()-2)+"/"+entry->d_name);
        cout<<current.substr(2,current.size()-2)+"/"+entry->d_name<<endl;
      } else {
        // result.push_back(CWD()+current+entry->d_name);
        cout<<current+entry->d_name<<endl;
      }

      vector<string> temp = fileListing(path);
      result.insert(result.end(), temp.begin(), temp.end());
    } else {
      if (current != "/") {
        result.push_back(CWD() + current.substr(2, current.size() - 2) + "/" +
                         entry->d_name);
        // cout<<CWD()+current.substr(2,current.size()-2)+"\\"+entry->d_name<<endl;
      } else {
        // cout<<CWD()+current+entry->d_name<<endl;
        result.push_back(CWD() + current + entry->d_name);
      }
    }
  }
  closedir(dir);
  return result;
}

void updateDatabase(const vector<string> & args){
  ofstream indata("VirusDatabase.dat");
  for (const string & x : args) {
    indata << x << endl;
  }
  indata.close();
}

int main() {
  //decrypt("/home/konri/Desktop/bso_antywirus/testFiles/malware.cpp");
  interface();
  vector<string> viruses = loadHashes();
  vector<string> quarantined = loadQuarantined();
  char choice ;
  char auxiliary;
  string toScan, toAdd;
  int scanned;
  int detected;
  int pom;

  bool scanRun;

  while (true) {
    cout << ">";
    cin >> choice;
    switch (choice) {
    case 'S':
      scanRun = true;
      cout << "wskaz plik lub folder do przeskanowania, " << endl << ">";
      cin >> toScan;
      if (toScan.empty())
        toScan = ".";
      scaning(toScan.c_str(), viruses, scanned, detected, quarantined);
      cout << "\033[1;33mSKANOWANIE ZAKONCZONE \033[0m" << endl;
      saveQuarantined(quarantined);
      cout << "\033[1;33mPLIKI DODANE DO KWARANTANNY \033[0m" << endl;

      break;
    case 'E':
      cout << "dziekuje za korzystanie z mojego programu" << endl;
      return 0;
    case 'T':
      if (scanRun) {
        cout << "oto statystyki" << endl;
        cout << "przeskanowano : " << scanned << endl;
        cout << "wirusow : " << detected << endl;
        cout << 100 * (double)detected / (double)scanned << "%" << endl;
      } else
        cout << "Narpierw prosze przeskanuj :)" << endl;
      break;
    case 'Q':
      if (quarantined.empty()) {
        cout << "obecnie zaden plik nie znajduje sie w kwarantannie" << endl;
        break;
      }
      cout << "pliki w kwarantannie" << endl;
      for (const string & x : quarantined)
        cout << x << endl;
      cout << "czy chcesz usunac jakis plik z kwarantanny? (Y/N)" << endl
           << ">";
      cin >> auxiliary;
      if (auxiliary == 'Y') {
        cout << "podaj numer pliku z listy powyzej" << endl << ">";
        cin >> pom;
        cout << "usunieto " << unquarantine(pom, quarantined)
             << " z kwarantanny" << endl;
        saveQuarantined(quarantined);
      }
      break;
    case 'A':
      cout << "podaj plik, ktory ma zostac dodany do bazy skrotow plikow szkodliwych" << endl << ">";
      cin >> toAdd;
      viruses.push_back(hashingMD5(toAdd.c_str()));
      updateDatabase(viruses);
      cout << "Dodano " << toAdd << " " << hashingMD5(toAdd.c_str()) << " do bazy wirusow" << endl;
      break;
    default:
      cout << "prosze korzystac z mozliwych opcji!!!" << endl;
      break;
    }
  }
  // vector<string> files = fileListing(".");
  // generateHashes(fileListing("."));
  // encrypt("/home/konri/Desktop/bso_antywirus/fileToEncode");
}

#pragma clang diagnostic pop