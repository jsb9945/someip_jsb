#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

// Some/IP 헤더 구조체 정의
struct someip_header_t {
    uint32_t message_id;
    uint32_t length;
    uint32_t request_id;
    uint8_t protocol_version;
    uint8_t interface_version;
    uint8_t message_type;
    uint8_t reserved;
    uint16_t method_id;
    uint16_t client_id;
    uint16_t session_id;
};

// Some/IP 통신을 담당하는 클래스 정의
class AutosarSomeIpServer {
public:
    AutosarSomeIpServer(int port_number, uint16_t service_id, uint16_t method_id) : service_id_(service_id), method_id_(method_id) {
        // 서버 주소와 포트 번호 설정
        memset(&server_address_, 0, sizeof(server_address_));
        server_address_.sin_family = AF_INET;
        server_address_.sin_addr.s_addr = INADDR_ANY;
        server_address_.sin_port = htons(port_number);

        // 소켓 생성
        if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            cerr << "Error: Failed to create socket" << endl;
        }

        // 소켓 바인딩
        if (bind(socket_, (struct sockaddr*)&server_address_, sizeof(server_address_)) < 0) {
            cerr << "Error: Failed to bind socket" << endl;
        }
    }

    void receive() {
        char buffer[1024];
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        ssize_t recv_len;

        while (true) {
            // Some/IP Message 수신
            recv_len = recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &client_address_len);
            if (recv_len < 0) {
                cerr << "Error: Failed to receive message" << endl;
                break;
            }

            // Some/IP Header 분석
            someip_header_t header;
            memcpy(&header, buffer, sizeof(header));
            if (header.method_id != method_id_) {
                cerr << "Error: Method ID does not match" << endl;
                continue;
            }

            // Some/IP Message 처리
            string message_data(buffer + sizeof(header), recv_len - sizeof(header));
            cout << "Received message: " << message_data << endl;
        }
    }

private:
    int socket_;
    struct sockaddr_in server_address_;
    uint16_t service_id_;
    uint16_t method_id_;
};

int main() {
    // Some/IP 서버 객체 생성
    AutosarSomeIpServer server(3000, 0x1234, 0x5678);

    // Some/IP Message 수신 및 처리
    server.receive();

    return 0;
}