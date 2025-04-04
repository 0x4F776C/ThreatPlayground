# Installation Steps

## 1. Install AD DS Role
```powershell
Install-WindowsFeature AD-Domain-Services -IncludeManagementTools
```

## 2. Promote Server to Domain Controller
```powershell
Install-ADDSForest -DomainName "example.local" -DomainMode "WinThreshold" -ForestMode "WinThreshold" -InstallDNS
```

## 3. Configure LDAPS
1. Request SSL certificate:
   ```powershell
   Get-Certificate -Template "WebServer" -DnsName "ldap.example.local" -CertStoreLocation "Cert:\LocalMachine\My"
   ```
2. Bind certificate to port 636:
   ```powershell
   netsh http add sslcert ipport=0.0.0.0:636 certhash=THUMBPRINT appid={00112233-4455-6677-8899-AABBCCDDEEFF}
   ```