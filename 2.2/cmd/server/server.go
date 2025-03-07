package main

import (
	"fmt"
	"net"
	"os"
)

func main() {
 // Define the TCP server address to listen on
 tcpServerAddress := "localhost:8080"

 // Create a TCP listener
 tcpListener, tcpListenerError := net.Listen("tcp", tcpServerAddress)
 if tcpListenerError != nil {
  fmt.Println("Error creating TCP listener:", tcpListenerError)
  os.Exit(1)
 }
 defer tcpListener.Close()

 fmt.Println("TCP server started. Listening on", tcpServerAddress)

 // Infinite loop to accept incoming connections
 for {
  // Accept incoming connection
  clientConnection, clientConnectionError := tcpListener.Accept()
  if clientConnectionError != nil {
   fmt.Println("Error accepting connection:", clientConnectionError)
   continue
  }

  fmt.Println("Client connected:", clientConnection.RemoteAddr())

  // Create a buffer to read data from the client
  clientDataBuffer := make([]byte, 1024)

  // Read data from the client
  dataLength, dataReadError := clientConnection.Read(clientDataBuffer)
  if dataReadError != nil {
   fmt.Println("Error reading data:", dataReadError)
   clientConnection.Close()
   continue
  }

  // Process the received data
  receivedMessage := string(clientDataBuffer[:dataLength])
  fmt.Println("Received message:", receivedMessage)

  // Send response back to the client
  responseMessage := "Server received your message: " + receivedMessage
  _, writeError := clientConnection.Write([]byte(responseMessage))
  if writeError != nil {
   fmt.Println("Error sending response:", writeError)
  }

  // Close the connection
  clientConnection.Close()
  fmt.Println("Connection closed with", clientConnection.RemoteAddr())
 }
}
