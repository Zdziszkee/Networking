package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
)

func main() {
 // Connect to server
 conn, err := net.Dial("tcp", "localhost:8080")
 if err != nil {
  fmt.Println("Error connecting:", err)
  os.Exit(1)
 }
 defer conn.Close()

 fmt.Println("Connected to server")

 // Create reader for user input
 reader := bufio.NewReader(os.Stdin)

 // Main communication loop
 for {
  // Read input from user
  fmt.Print("Enter message: ")
  text, err := reader.ReadString('\n')
  if err != nil {
   fmt.Println("Error reading input:", err)
   break
  }

  // Send to server
  _, err = fmt.Fprintf(conn, text)
  if err != nil {
   fmt.Println("Error sending data:", err)
   break
  }

  // Read response from server
  message, err := bufio.NewReader(conn).ReadString('\n')
  if err != nil {
   fmt.Println("Error reading response:", err)
   break
  }

  fmt.Print("Server response: " + message)
 }
}
