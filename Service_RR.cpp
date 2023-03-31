#include <cstdint>
#include <vector>

// SOME/IP Header 구조체
struct SomeIpHeader {
    uint16_t service_id_;
    uint16_t method_id_;
    uint32_t length_;
    uint16_t client_id_; // request_id 앞 16bit
    uint16_t session_id_; // request_id 뒤 16bit
    uint8_t protocol_version_ = 1; // 생성자를 통해 1로 초기화       
    uint8_t interface_version_; // 1으로 통일?
    uint8_t message_type_; // request인지 response인지 구분
    uint8_t return_code_;
};

// Request & Response 호출 구조체
struct Service_RR {
    // request 생성 함수 선언
    SomeIpMessage CreateRequest(uint16_t service_id, uint16_t method_id, 
                                uint16_t client_id, uint16_t session_id,
                                const std::vector<uint8_t>& payload);
    // response 생성 함수 선언
    SomeIpMessage CreateResponse(const SomeIpMessage& request,
                                const std::vector<uint8_t>& payload,
                                uint16_t service_id, uint16_t method_id,
                                uint16_t session_id,
                                uint8_t return_code);    
};

// SOME/IP 메세지 생성 클래스
class SomeIpMessage {
    public:
        SomeIpHeader header_; // 헤더
        std::vector<uint8_t> payload_; // 페이로드

        SomeIpMessage() {
            header_.service_id_ = 0;  header_.length_ = 0; header_.client_id_ = 0;
            header_.session_id_ = 0; header_.protocol_version_ = 1; header_.message_type_ = 0;
            header_.interface_version_ = 1; header_.return_code_ = 0;
        }

};

// request 생성 함수 정의
SomeIpMessage Service_RR::CreateRequest(uint16_t service_id, uint16_t method_id, uint16_t client_id, uint16_t session_id, const std::vector<uint8_t>& payload){
    // some/ip request message
    SomeIpMessage message;
    message.header_.service_id_ = service_id; // message_id 앞 16비트
    message.header_.method_id_ = method_id; // message_id 뒤 16비트
    message.header_.length_ = static_cast<uint32_t>(payload.size() + 64);
    message.header_.client_id_ = client_id; // request id 앞 16bit
    message.header_.session_id_ = session_id; // request id 뒤 16bit    
    message.header_.message_type_ = 0x00; // message type이 0x00이면 request를 뜻함
    message.header_.protocol_version_ = 1;
    message.header_.interface_version_ = 1;
    
    // payload 구성
    message.payload_.reserve(payload.size()); // payload 사이즈 미리 확보
    message.payload_.insert(message.payload_.end(), payload.begin(), payload.end()); // 매개변수로 받는 payload 더하기

    return message;
}

// response 생성 함수 정의
SomeIpMessage Service_RR::CreateResponse(const SomeIpMessage& request, const std::vector<uint8_t>& payload, uint16_t service_id, uint16_t method_id, uint16_t session_id, uint8_t return_code){
    // some/ip response message
    SomeIpMessage response;
    response.header_.service_id_ = service_id; // client가 보낸 service_id 그대로 리턴
    response.header_.method_id_ = method_id; // client가 보낸 method_id 그대로 리턴
    response.header_.length_ = static_cast<uint32_t>(payload.size() + 64);
    response.header_.session_id_ = session_id;
    response.header_.message_type_ = 0x80; // message type이 0x80이면 response를 뜻함
    response.header_.protocol_version_ = 1;
    response.header_.interface_version_ = 1;
    response.header_.return_code_ = return_code;

    // payload 구성
    response.payload_.reserve(payload.size());
    response.payload_ = payload;

    return response;
}

// 형태만 있으니깐 사용할 용도
// 시나리오를 정해서 그걸 구현해라
// 서버랑 클라이언트 나눠서 코드 구현하고 거기에 some/ip 넣으면 되는 것