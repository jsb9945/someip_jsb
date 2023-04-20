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
class AutosarSomeIpClient {
public:
    AutosarSomeIpClient(const char* ip_address, int port_number, uint16_t service_id, uint16_t method_id) : service_id_(service_id), method_id_(method_id) {
        // 서버 주소와 포트 번호 설정
        memset(&server_address_, 0, sizeof(server_address_));
        server_address_.sin_family = AF_INET;
        server_address_.sin_addr.s_addr = inet_addr(ip_address);
        server_address_.sin_port = htons(port_number);
        
        // 소켓 생성
        if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            cerr << "Error: Failed to create socket" << endl;
        }
    }

    void send(const string& message_data) {
        // Some/IP Message 생성
        someip_header_t header;
        memset(&header, 0, sizeof(header));
        header.length = message_data.length() + sizeof(header);
        header.protocol_version = 1;
        header.interface_version = 1;
        header.message_type = 0;
        header.method_id = method_id_;
        header.client_id = 0;
        header.session_id = 0;
        
        char buffer[sizeof(header) + message_data.length()];
        memcpy(buffer, &header, sizeof(header));
        memcpy(buffer + sizeof(header), message_data.c_str(), message_data.length());

        // Some/IP Message를 서버에 전송합니다.
        if (sendto(socket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_address_, sizeof(server_address_)) < 0) {
            cerr << "Error: Failed to send message" << endl;
        }
    }

private:
    int socket_;
    struct sockaddr_in server_address_;
    uint16_t service_id_;
    uint16_t method_id_;
};

int main() {
    // Some/IP 클라이언트 객체 생성
    AutosarSomeIpClient client("192.168.0.2", 3000, 0x1234, 0x5678);

    // Some/IP Message 전송
    client.send("Hello, world!");

    return 0;
}
