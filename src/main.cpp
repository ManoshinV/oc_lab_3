#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <sys/types.h>
#include <sys/mman.h> 
using namespace std;

const int shsize = 1024;

int main() {
    string name;
    cin >> name;
    ofstream out(name);

    int fd_input = shm_open("input", O_CREAT | O_RDWR, 0666);
    int fd_error = shm_open("error", O_CREAT | O_RDWR, 0666);

    if (fd_input == -1 || fd_error == -1) {
        shm_unlink("input");
        shm_unlink("error");
        cout << "open error" << endl;
        return 1;
    }

    char* shared_input = (char*)mmap(NULL, shsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_input, 0);
    char* shared_error = (char*)mmap(NULL, shsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_error, 0);

    if (ftruncate(fd_error, shsize) == -1) {
        shm_unlink("input");
        shm_unlink("error");
        munmap(shared_input, shsize);
        munmap(shared_error, shsize);
        cout << "mmap error" << endl;
        return 2;
    }
    close(fd_input);
    close(fd_error);
    int t;
    cin >> t;
    vector<vector<double>>dt(t);
    for (int cs = 0; cs < t; ++cs) {
        int n;
        cin >> n;
        dt[cs].resize(n);
        for (double& x : dt[cs]) cin >> x;
    }
    int pid = fork();
    if (pid == 0) {
        string Err;
        for (auto& cs : dt) {
            double res = cs[0];
            for (int i = 1; i < cs.size(); ++i) {
                if (cs[i] == 0.0) {
                    Err += "zerro error\n";
                    strcpy(shared_error, Err.c_str());
                    return -1;
                }
                res /= cs[i];
            }
            out << res << '\n';
        }
        Err += '\n';
        strcpy(shared_error, Err.c_str());
    }
    if (pid > 0) {
        wait(0);
        for (int i = 0; i < shsize && shared_error[i] != '\n'; ++i) {
            cout << shared_error[i];
        }
    }
    if (pid == -1) {
        shm_unlink("input");
        shm_unlink("error");
        munmap(shared_input, shsize);
        munmap(shared_error, shsize);
        cout << "pid error" << endl;
        return 0;
    }
    return 0;
}
