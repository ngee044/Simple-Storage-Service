CREATE TABLE IF NOT EXISTS s3_buckets (
    id SERIAL PRIMARY KEY,
    bucket_name VARCHAR(255) UNIQUE NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS s3_objects (
    id SERIAL PRIMARY KEY,
    bucket_id INT NOT NULL,
    object_key VARCHAR(1024) NOT NULL,
    file_size BIGINT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMP NOT NULL DEFAULT NOW(),
    CONSTRAINT fk_bucket
        FOREIGN KEY (bucket_id)
        REFERENCES s3_buckets(id)
        ON DELETE CASCADE
);
        