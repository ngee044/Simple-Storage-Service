package models

type TaskMessage struct {
	Action     string `json:"action"`
	RequestID  string `json:"request_id"`
	FileName   string `json:"filename,omitempty"`
	BucketName string `json:"bucket_name,omitempty"`
}

type BucketRequest struct {
	BucketName string `json:"bucket_name"`
}

type FileRequest struct {
	Filename string `json:"filename"`
}
