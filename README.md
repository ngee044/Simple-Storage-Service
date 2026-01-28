# Simple-Storage-Service

AWS S3 호환 스토리지 시스템을 마이크로서비스 아키텍처로 구현한 포트폴리오 프로젝트입니다.

## 주요 기술 스택

| 컴포넌트 | 기술 |
|---------|------|
| REST API | Go 1.24, Gin Framework |
| Core Service | C++20, CMake, vcpkg |
| Message Queue | RabbitMQ |
| Cache | Redis |
| Database | PostgreSQL |
| Container | Docker, Docker Compose |

---

## Quick Start (Docker)

```bash
# 1. 저장소 클론
git clone https://github.com/your-repo/Simple-Storage-Service.git
cd Simple-Storage-Service

# 2. 서브모듈 초기화
git submodule update --init --recursive

# 3. Docker 실행
cd Docker
docker-compose up -d

# 4. 상태 확인
docker-compose ps
```

### 서비스 접속 정보

| 서비스 | URL | 설명 |
|--------|-----|------|
| REST API | http://localhost:8080 | API 엔드포인트 |
| RabbitMQ UI | http://localhost:15672 | 관리 UI (guest/guest) |
| PostgreSQL | localhost:5432 | DB (root/root) |
| Redis | localhost:6379 | 캐시 |

---

## 로컬 개발 환경 설정

### Prerequisites

- Go 1.24+
- CMake 3.18+
- vcpkg
- PostgreSQL 15+
- Redis 7+
- RabbitMQ 3+

### Go REST API 빌드

```bash
cd S3RestAPI

# 의존성 설치
go mod download

# 빌드
./build.sh

# 실행
APP_ENV=development ./bin/RestAPI
```

### C++ Core Service 빌드

```bash
# vcpkg 설치 (최초 1회)
git clone https://github.com/Microsoft/vcpkg.git ../vcpkg
../vcpkg/bootstrap-vcpkg.sh

# 빌드
./build.sh

# 실행
cd build/out
./S3Service
```

### 환경 변수 설정

**S3RestAPI/.env.development**
```env
RABBITMQ_DSN=amqp://guest:guest@localhost:5672/
RABBITMQ_QUEUE=request_message
REDIS_ADDR=localhost:6379
REDIS_PASS=
SERVER_PORT=8080
JWT_SECRET=your-secret-key-change-in-production
```

---

## API 엔드포인트

### 인증

모든 API 엔드포인트 (`/health`, `/login`, `/register` 제외)는 JWT 토큰 인증이 필요합니다.

```bash
# 1. 사용자 등록 (선택사항 - 기본 계정: admin/admin)
curl -X POST http://localhost:8080/register \
  -H "Content-Type: application/json" \
  -d '{"username":"newuser","password":"mypassword"}'

# 2. 로그인하여 JWT 토큰 발급
TOKEN=$(curl -s -X POST http://localhost:8080/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin"}' | jq -r '.token')

# 발급된 토큰 확인
echo $TOKEN
```

### 버킷 관리

```bash
# 버킷 생성 (인증 필요)
curl -X POST http://localhost:8080/buckets \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"bucket_name":"my-bucket"}'

# 버킷 내 파일 목록 (인증 필요)
curl http://localhost:8080/buckets/my-bucket/files \
  -H "Authorization: Bearer $TOKEN"
```

### 파일 관리

```bash
# 파일 업로드 (인증 필요)
curl -X POST http://localhost:8080/buckets/my-bucket/files \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"filename":"test.txt"}'

# 파일 다운로드 (인증 필요)
curl http://localhost:8080/buckets/my-bucket/files/test.txt/download \
  -H "Authorization: Bearer $TOKEN"

# 파일 삭제 (인증 필요)
curl -X DELETE http://localhost:8080/buckets/my-bucket/files/test.txt \
  -H "Authorization: Bearer $TOKEN"
```

### 작업 상태 확인

```bash
# 비동기 작업 상태 조회 (인증 필요)
curl http://localhost:8080/tasks/{request_id}/status \
  -H "Authorization: Bearer $TOKEN"
```

### 헬스체크

```bash
# 헬스체크 (인증 불필요)
curl http://localhost:8080/health
```

---

## 아키텍처

### 전체 구조

```mermaid
flowchart TB
    A[Client<br>Web UI, CLI] -->|HTTP/HTTPS| B[Go REST API<br>Port 8080]
    B -->|Publish| MQ[RabbitMQ<br>Port 5672]
    MQ -->|Consume| C[C++ Core Service]
    C --> D[(PostgreSQL<br>Port 5432)]
    C --> E[(File Storage)]
    C -->|Store Result| R[(Redis<br>Port 6379)]
    B -->|Poll Result| R

    style A fill:#CDE5FA,stroke:#0081C2
    style B fill:#FAF3CA,stroke:#E0C408
    style C fill:#FAE1D3,stroke:#E38B16
    style D fill:#D3FAD3,stroke:#4EB449
    style E fill:#FEE2E2,stroke:#EA5555
    style MQ fill:#E8E8E8,stroke:#888888
    style R fill:#FFE4E4,stroke:#FF6B6B
```

### 비동기 요청 처리 흐름

1. 클라이언트가 REST API에 요청
2. REST API가 `request_id` 생성 후 RabbitMQ에 메시지 발행
3. 클라이언트에게 HTTP 202 (Accepted) + `request_id` 즉시 반환
4. C++ Core Service가 RabbitMQ에서 메시지 소비
5. 처리 완료 후 결과를 Redis에 저장 (key: `request_id`)
6. 클라이언트가 `/tasks/{request_id}/status`로 결과 조회

### 버킷 생성 시퀀스

```mermaid
sequenceDiagram
    participant U as Client
    participant G as Go REST API
    participant MQ as RabbitMQ
    participant C as C++ Core
    participant DB as PostgreSQL
    participant R as Redis

    U->>G: POST /buckets
    G->>MQ: Publish (request_id, action, bucket_name)
    G-->>U: 202 Accepted + request_id
    MQ->>C: Consume message
    C->>DB: INSERT bucket
    DB-->>C: OK
    C->>R: SET request_id = result
    U->>G: GET /tasks/{request_id}/status
    G->>R: GET request_id
    R-->>G: result
    G-->>U: 200 OK + result
```

---

## 프로젝트 구조

```
Simple-Storage-Service/
├── S3RestAPI/              # Go REST API
│   ├── cmd/restapi/        # 메인 진입점
│   ├── internal/
│   │   ├── auth/           # 사용자 인증 (bcrypt)
│   │   ├── config/         # 설정 관리 (JWT Secret)
│   │   ├── controllers/    # HTTP 핸들러
│   │   ├── middleware/     # JWT 인증 미들웨어
│   │   ├── models/         # 요청/응답 모델
│   │   └── transport/      # RabbitMQ, Redis 클라이언트
│   └── .env.development
├── S3Service/              # C++ Core Service
│   ├── S3Service.cpp       # 메인 서비스 로직
│   ├── Configurations.cpp  # 설정 관리
│   └── s3_service_configurations.json
├── S3FileStorage/          # 파일 스토리지 모듈
├── S3MetaDB/               # 메타데이터 DB 모듈
├── S3Common/               # 공통 유틸리티
├── Docker/                 # Docker 설정
│   ├── docker-compose.yaml
│   ├── Dockerfile.restapi
│   └── Dockerfile.core
├── .cpp_tool_kit/          # C++ 유틸리티 (서브모듈)
├── CMakeLists.txt          # C++ 빌드 설정
├── vcpkg.json              # C++ 패키지 의존성
└── build.sh                # C++ 빌드 스크립트
```

---

## 데이터베이스 스키마

```sql
-- 버킷 테이블
CREATE TABLE s3_buckets (
    id SERIAL PRIMARY KEY,
    bucket_name VARCHAR(255) UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT NOW()
);

-- 오브젝트 테이블
CREATE TABLE s3_objects (
    id SERIAL PRIMARY KEY,
    bucket_id INT NOT NULL REFERENCES s3_buckets(id) ON DELETE CASCADE,
    object_key VARCHAR(1024) NOT NULL,
    file_size BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);
```

---

## 설정 파일

### C++ Core Service (s3_service_configurations.json)

| 설정 | 설명 | 기본값 |
|-----|------|--------|
| database_server_ip | PostgreSQL 호스트 | 127.0.0.1 |
| database_server_port | PostgreSQL 포트 | 5432 |
| rabbit_mq_host | RabbitMQ 호스트 | localhost |
| rabbit_mq_port | RabbitMQ 포트 | 5672 |
| redis_host | Redis 호스트 | localhost |
| redis_port | Redis 포트 | 6379 |
| consume_queue_name | 메시지 큐 이름 | request_message |
| storage_path | 파일 저장 경로 | (실행 디렉토리)/test_storage/ |

---

## 라이선스

MIT License
