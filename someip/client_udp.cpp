#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

// Some/IP 헤더 구조체 정의
struct someip_header_t {
    //uint32_t message_id;
    uint32_t length;
    uint32_t request_id;
    uint8_t protocol_version;
    uint8_t interface_version;
    uint8_t message_type;
    //uint8_t reserved;
    uint16_t method_id;
    uint16_t client_id;
    uint16_t session_id;
};

// 위도, 경도 구조체
struct location_t {
    double latitude;
    double longitude;
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


    void send(const location_t& message_data) {
        // Some/IP Message 생성
        someip_header_t header;
        memset(&header, 0, sizeof(header));
        header.length = sizeof(message_data) + sizeof(header);
        header.protocol_version = 1;
        header.interface_version = 1;
        header.request_id = 8;
        header.message_type = 0x00; // Request
        header.method_id = method_id_;
        header.client_id = 0;
        header.session_id = 0;
        
        char buffer[sizeof(header) + sizeof(message_data)];
        memcpy(buffer, &header, sizeof(header));
        memcpy(buffer + sizeof(header), &message_data, sizeof(message_data));

        // Some/IP Message를 서버에 전송합니다.
        if (sendto(socket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_address_, sizeof(server_address_)) < 0) {
            cerr << "Error: Failed to send message" << endl;
        }

        char recv_buffer[1024];
        socklen_t server_address_len = sizeof(server_address_);
        ssize_t recv_len = recvfrom(socket_, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&server_address_, &server_address_len);
        if(recv_len<0){
            cerr << "Error: Failed to receive message" << endl;
        }
        else{
            someip_header_t recv_header;
            memcpy(&recv_header, recv_buffer, sizeof(recv_header));
            if(recv_header.request_id != header.request_id){
                cerr << "Error: Not same request_id" << endl;
            }
            else{
                string recv_data(recv_buffer + sizeof(recv_header), recv_len - sizeof(recv_header));
                cout << "Recv : " << recv_data << endl;    
            }
    
        }
    }


private:
    int socket_;
    struct sockaddr_in server_address_;
    uint16_t service_id_;
    uint16_t method_id_;
};

int main() {
    double lat, lon;
    location_t data;

    // Some/IP 클라이언트 객체 생성
    AutosarSomeIpClient client("192.168.0.153", 3000, 0x1234, 0x5678);

    // Some/IP Message 전송
    while(1){
        location_t data = {0.0, 0.0};
        cout << "lat: ";
        cin >> lat;
        data.latitude = lat;
        cout << "lon: ";
        cin >> lon;
        data.longitude = lon;
        
        client.send(data);

    }
    

    return 0;
}