#include "serverConnection.h"

#include <fstream>


void err_print(const char* nachricht) {
    fprintf(stderr, "%s\n", nachricht);
}

void err_close() {
    WSACleanup();
    exit(-1);
}

serverConnection::serverConnection(int port) {
    this->s_port = port;
    failed = false;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        setFail();
    }

    startConn();
}
serverConnection::~serverConnection() {
    if (WSACleanup() != 0) {
        err_print("Fail bei Cleanup");
    }

}

void serverConnection::setFail() {
    this->failed = true;
}
bool serverConnection::getFail() {
    return this->failed;
}
void serverConnection::startConn() throw(networkIOException) {
    conn_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_sock == INVALID_SOCKET) {
        err_print("Socket Fail");
        setFail();
        throw networkIOException("Socket creation failed");
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(s_port);
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(conn_sock, (struct sockaddr*) &dest_addr, sizeof(dest_addr)) == SOCKET_ERROR) {
        err_print("Bind Fail");
        setFail();
        throw networkIOException("Bind Fail");

    }

    if (listen(conn_sock, 5) == SOCKET_ERROR)
    {
        throw networkIOException("Listen failed");
    }

}
void serverConnection::stopConn() throw(networkIOException) {
    if (closesocket(conn_sock) == SOCKET_ERROR) {
        err_print("Fehler bei Close");
        setFail();
        throw networkIOException("Socket close failed");
    }
}

void serverConnection::acceptClient() throw(networkIOException) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(sockaddr);   // Init nötig, da sonst accept failed

    //client_sock = accept(conn_sock, (struct sockaddr*) &client_addr, &client_addr_len);
    client_sock = accept(conn_sock, (struct sockaddr*) &client_addr, &client_addr_len);

    if (client_sock == INVALID_SOCKET)
    {
        printf("CODE: %d\n", WSAGetLastError());
        throw networkIOException("Failed to accept client");
    }
    client_ip = inet_ntoa(client_addr.sin_addr);
}

void serverConnection::closeClient() throw(networkIOException) {
    if (closesocket(client_sock) == SOCKET_ERROR) {
        throw networkIOException("Failed to close client socket");
    }
}

const char* serverConnection::getClientIP()
{
    return client_ip.c_str();
}


int serverConnection::sendBuffer(const char* buffer, int buffSize, int chunkSize) throw(networkIOException) {
    int i = 0;

    while (i < buffSize) {
        int l = send(this->client_sock, &buffer[i], __min(chunkSize, buffSize - i), 0);
        if (l < 0) {
            err_print("sendBuffer Fail");   // Error
            throw networkIOException("Failed to send Buffer");
            return -1;
        }
        i += l;
    }
    return i;
}
int serverConnection::recvBuffer(char* buffer, int buffSize, int chunkSize) throw(networkIOException) {
    int i = 0;

    while (i < buffSize) {
        const int l = recv(this->client_sock, &buffer[i], __min(chunkSize, buffSize - i), 0);
        if (l < 0)
        {
            throw networkIOException("Failed to receive Buffer");
            return l;
        } // this is an error
        i += l;
    }
    return i;
}
int64_t serverConnection::getFileSize(const char* filename) throw(fileIOException) {
    printf("Overflow: %d\n", errno);
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        err_print("GetSize FileOpen Fail");
        throw fileIOException("GetSize FileOpen Fail");
        return -1;
    }

    if (fseek(file, 0L, SEEK_END) < 0) {
        err_print("GetSize Fail");
        fclose(file);
        throw fileIOException("GetSize seek failed");
        return -1;
    }
    errno = 0;
    long fsize = ftell(file);
    if (fsize < 0) {
        err_print("GetSize Fail");
        printf("Overflow: %d\n", errno);
        fclose(file);
        throw fileIOException("FileSize bigger than LONG_MAX");
        return -2;  // Datei > LONG_MAX
    }
    fclose(file);
    return fsize;
}
int serverConnection::sendFile(const char* filename, int chunkSize) throw(networkIOException, fileIOException) {
    // Dateigröße schicken:
    int64_t fileSize = getFileSize(filename);
    if (fileSize < 0) {
        if (fileSize == -2) {
            std::stringstream ss;
            ss << "Dateigröße von [" << filename << "] größer als MAX_LONG";
            printf("Dateigröße von [%s] größer als MAX_LONG\n", filename);
            throw networkIOException(ss.str().c_str());
        }
        else {  // Abgedeckt von getFileSize exception
            //throw;
            return -1;
        }
    }

    if (sendBuffer(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize)) != sizeof(fileSize)) {
        throw networkIOException("sending FileSize failed");
        return -1;
    }

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        throw fileIOException("opening File failed");
        return -2;
    }

    char* buffer = new char[chunkSize];
    bool errored = false;
    int64_t i = fileSize;
    while (i != 0) {
        const int64_t ssize = __min(i, (int64_t)chunkSize);
        if (fread(buffer, sizeof(char), ssize, file) < ssize) { errored = true; break; }
        const int l = sendBuffer(buffer, (int)ssize);
        if (l < 0) { errored = true; break; }
        i -= l;
    }
    delete[] buffer;

    fclose(file);

    //return errored ? -3 : fileSize;

    if (errored)
    {
        throw networkIOException("sending file failed");
        return -3;
    }
    else
    {
        return fileSize;
    }
}
int serverConnection::recvFile(const char* filename, int chunkSize) {
    std::ofstream file(filename, std::ofstream::binary);
    if (file.fail())
    {
        throw fileIOException("Failed to open file for receive");
        return -1;
    }

    /*FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        throw fileIOException("Failed to open File for receive");
    }*/

    int64_t fileSize;
    if (recvBuffer(reinterpret_cast<char*>(&fileSize), sizeof(fileSize)) != sizeof(fileSize)) {
        throw networkIOException("Failed to receive file size");
        return -2;
    }

    char* buffer = new char[chunkSize];
    bool errored = false;
    int64_t i = fileSize;
    while (i != 0) {        // Datei senden
        const int r = recvBuffer(buffer, (int)__min(i, (int64_t)chunkSize));
        if ((r < 0) || !file.write(buffer, r)) { errored = true; break; }
        i -= r;
    }
    delete[] buffer;

    file.close();

    //return errored ? -3 : fileSize;

    if (errored)
    {
        throw networkIOException("failed to receive file");
    }
    else
    {
        return fileSize;
    }
}