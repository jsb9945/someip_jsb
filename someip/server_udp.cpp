#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cmath>

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

double degreeToRadian(double degree);
double radianToDegree(double radian);
double getBearing(double lat1, double lng1, double lat2, double lng2);

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
            location_t message_data;
            memcpy(&message_data, buffer + sizeof(header), sizeof(message_data));
            
            double lat = message_data.latitude;
            double lon = message_data.longitude;
            double target_lat = 37.5665;
            double target_lon = 126.9780;
            cout << "latitude: " << lat << "\tlongitude: " << lon << endl;
            //여기까진 문제 없음


            // 위도 경도로 방향 알려주는 알고리즘
            double bearing = getBearing(lat, lon, target_lat, target_lon);
            string message_data1 = to_string(bearing);

            //string message_data1 = "Hello World";
            send_response(socket_, &client_address, client_address_len, header.request_id, message_data1);
        }
    }

    void send_response(int socket, struct sockaddr_in* client_address_, socklen_t client_address_len_, uint32_t request_id_, string& message_data_) {
        // Some/IP Response Message 구성
        someip_header_t header;
        memset(&header, 0, sizeof(header));
        //header.message_id = message_id_;
        header.length = sizeof(header) + sizeof(message_data_);
        //header.request_id = request_id_;
        header.request_id = request_id_;
        header.protocol_version = 1;
        header.interface_version = 1;
        header.message_type = 0x02;  // Response
        header.method_id = 0;
        header.client_id = 0;
        header.session_id = 0;

        // Some/IP Response Message 전송
        char buffer[1024];
        memset(&buffer, 0, sizeof(buffer));
        memcpy(buffer, &header, sizeof(header));
        memcpy(buffer + sizeof(header), message_data_.c_str(), message_data_.length());
        if (sendto(socket, buffer, header.length, 0, (struct sockaddr*)client_address_, client_address_len_) < 0) {
            cerr << "Error: Failed to send response message" << endl;
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

double degreeToRadian(double degree) {
    return degree * M_PI / 180.0;
}

double radianToDegree(double radian) {
    return radian * 180.0 / M_PI;
}

double getBearing(double lat1, double lng1, double lat2, double lng2) {
    double dLon = degreeToRadian(lng2 - lng1);
    double y = sin(dLon) * cos(degreeToRadian(lat2));
    double x = cos(degreeToRadian(lat1)) * sin(degreeToRadian(lat2)) -
               sin(degreeToRadian(lat1)) * cos(degreeToRadian(lat2)) * cos(dLon);
    double bearing = radianToDegree(atan2(y, x));
    if (bearing < 0) {
        bearing += 360.0;
    }
    return bearing;
}