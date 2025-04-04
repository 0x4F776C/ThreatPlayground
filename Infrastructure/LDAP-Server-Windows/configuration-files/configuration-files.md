# Configuration Files

## 1. LDAP Client Configuration (`ldap.conf`)
```
BASE    dc=example,dc=local
URI     ldaps://ldap.example.local
SSL     on
TLS_REQCERT demand
```

## 2. Group Policy Settings
```xml
<LDAPSettings>
  <SecurityDescriptor>D:(A;;RP;;;AU)</SecurityDescriptor>
  <MaxPageSize>1000</MaxPageSize>
</LDAPSettings>
```