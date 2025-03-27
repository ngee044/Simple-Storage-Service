package transport

import (
	"encoding/json"
	"fmt"

	"github.com/streadway/amqp"
)

var rabbitConn *amqp.Connection
var rabbitChannel *amqp.Channel
var queueName string

func SetupRabbitMQ(dsn, qName string) error {
	var err error

	rabbitConn, err = amqp.Dial(dsn)
	if err != nil {
		return fmt.Errorf("Failed to connect to RabbitMQ: %v", err)
	}

	rabbitChannel, err = rabbitConn.Channel()
	if err != nil {
		return fmt.Errorf("Failed to open a channel: %v", err)
	}

	_, err = rabbitChannel.QueueDeclare(
		qName,
		true,  // durable
		false, // autoDelete
		false, // exclusive
		false, // noWait
		nil,
	)
	if err != nil {
		return fmt.Errorf("RabbitMQ 큐 선언 실패: %w", err)
	}
	queueName = qName

	return nil
}

func CloseRabbitMQ() {
	if rabbitChannel != nil {
		rabbitChannel.Close()
	}

	if rabbitConn != nil {
		rabbitConn.Close()
	}
}

func PublishMessage(payload interface{}, guid string) error {
	body, err := json.Marshal(payload)
	if err != nil {
		return fmt.Errorf("Failed to marshal payload: %v", err)
	}

	err = rabbitChannel.Publish(
		"",
		queueName,
		false,
		false,
		amqp.Publishing{
			ContentType:   "application/json",
			Body:          body,
			CorrelationId: guid,
			DeliveryMode:  amqp.Persistent,
		},
	)
	if err != nil {
		return fmt.Errorf("Failed to publish a message: %v", err)
	}
	return nil
}
