# Simple-Storage-Service
Developing the main functions of AWS S3 in a portfolio format

Developing as simple as possible for demonstration purposes, but developing in a form that is close to practice and adaptable

Developing in MSA format.

Developing with a focus on functionality.

This is an expanded portfolio of the RealtimeMessageChat portfolio. Developing so that it can be applied to practice depending on the user.
---
## Overall architecture (Flowchart)

- **Golang REST API**: Receives HTTP requests from outside and processes them (creates buckets, uploads/downloads objects, etc.)
- Separate interfaces such as separate RestAPIs or Applications are possible
- **C++ Core**: Performs actual storage logic (saves files, processes metadata, etc.)
- Choose C++ because it is fast
- **Meta DB**: Stores buckets and object metadata (RDB or NoSQL)
- Plans to use PostgreSQL, Redis
- **File Storage**: Actual file (object) storage (directory, NAS, or simple block storage, etc.)
- Plans to use a directory

```mermaid
flowchart TB
    A[Client<br> Web UI, CLI ETC] -->|HTTP/HTTPS| B[Golang<br> REST API]
   
    B -->|RabbitMQ -> gRPC| C[C++ Core<br> Storage Logic]
    C --> D[(Meta DB)]
    C --> E[(File Storage)]

    style A fill:#CDE5FA,stroke:#0081C2,stroke-width:2px
    style B fill:#FAF3CA,stroke:#E0C408,stroke-width:2px
    style C fill:#FAE1D3,stroke:#E38B16,stroke-width:2px
    style D fill:#D3FAD3,stroke:#4EB449,stroke-width:2px
    style E fill:#FEE2E2,stroke:#EA5555,stroke-width:2px
```
---
### Create Bucket

```mermaid
sequenceDiagram
    participant U as User(Client)
    participant G as Golang REST API
    participant C as C++ Core
    participant DB as Meta DB
    
    U->>G: POST /buckets (버킷 생성 요청)
    G->>C: createBucket(bucketName)
    C->>DB: INSERT bucket metadata
    DB-->>C: success
    C-->>G: Bucket 생성 결과(성공/실패)
    G-->>U: HTTP 200 OK (버킷 생성 완료)

```
---
### Upload Object

```mermaid
sequenceDiagram
    participant U as User(Client)
    participant G as Golang REST API
    participant C as C++ Core
    participant DB as Meta DB
    participant FS as File Storage
    
    U->>G: PUT /buckets/{bucketName}/objects/{objectKey} (파일 업로드)
    G->>C: uploadObject(bucketName, objectKey, fileData)
    alt 버킷 존재 확인
        C->>DB: SELECT bucket metadata
        DB-->>C: bucket info
    else 버킷 미존재 시
        C-->>G: Error(버킷 없음)
        G-->>U: 404 Not Found
    end
    C->>FS: write fileData
    FS-->>C: success
    C->>DB: INSERT or UPDATE object metadata
    DB-->>C: success
    C-->>G: Upload success
    G-->>U: HTTP 200 OK
```
---
### Download Object
```mermaid
sequenceDiagram
    participant U as User(Client)
    participant G as Golang REST API
    participant C as C++ Core
    participant DB as Meta DB
    participant FS as File Storage

    U->>G: GET /buckets/{bucketName}/objects/{objectKey}
    G->>C: getObject(bucketName, objectKey)
    alt 버킷/오브젝트 존재 확인
        C->>DB: SELECT object metadata
        DB-->>C: object info
    else 미존재 시
        C-->>G: Error(오브젝트 없음)
        G-->>U: 404 Not Found
    end
    C->>FS: read fileData
    FS-->>C: fileData
    C-->>G: fileData
    G-->>U: HTTP 200 OK + fileData
```
---
### Summary

- Visualize the entire MVP configuration (Client → Golang REST API → C++ Core → DB & File Storage) with **Flowchart**
- Express the flow of main functions (Bucket creation, Object upload, Object download) with **Sequence Diagram**
- Implement the MVP scope (Bucket creation/deletion, Object upload/download, Simple authentication, etc.) step by step based on this structure