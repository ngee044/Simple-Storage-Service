# Docker 실행 가이드

## 빠른 시작

```bash
cd Docker
docker-compose up -d
```

## 서비스 목록

| 서비스 | 포트 | 설명 |
|--------|------|------|
| rest-api | 8080 | Go REST API |
| core-service | - | C++ Core Service (내부) |
| rabbitmq | 5672, 15672 | 메시지 큐 (관리UI: 15672) |
| redis | 6379 | 결과 캐시 |
| postgres | 5432 | 메타데이터 DB |

## 명령어

```bash
# 전체 서비스 시작
docker-compose up -d

# 로그 확인
docker-compose logs -f

# 특정 서비스 로그
docker-compose logs -f rest-api
docker-compose logs -f core-service

# 서비스 중지
docker-compose down

# 볼륨 포함 완전 삭제
docker-compose down -v

# 재빌드
docker-compose up -d --build
```

## 접속 정보

### RabbitMQ 관리 UI
- URL: http://localhost:15672
- ID/PW: guest/guest

### PostgreSQL
- Host: localhost:5432
- Database: s3_service
- User: root
- Password: root

### REST API
- Base URL: http://localhost:8080
- Health Check: GET /health

## API 테스트

```bash
# Health Check (인증 불필요)
curl http://localhost:8080/health

# Login (JWT 토큰 발급)
TOKEN=$(curl -s -X POST http://localhost:8080/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin"}' | jq -r '.token')

# 토큰 확인
echo $TOKEN

# Create Bucket (인증 필요)
curl -X POST http://localhost:8080/buckets \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"bucket_name":"my-bucket"}'

# Check Task Status (인증 필요)
curl http://localhost:8080/tasks/{request_id}/status \
  -H "Authorization: Bearer $TOKEN"
```
