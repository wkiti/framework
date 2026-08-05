### File 3: `docs/04_SECURITY_AND_HTTPS.md`

```markdown
# 🔒 Security, Hashing, & HTTPS

Wkiti follows "Defense in Depth" principles, enforcing security at the input, application, transport, and storage levels.

---

## 1. Password Hashing (OpenSSL SHA-256)

Plaintext passwords are never written to the database. Wkiti uses OpenSSL's `EVP_sha256()` message digest API with framework salting.

### Hashing Mechanism (`Wkiti::Security::hash_password`)

```cpp
std::string hashed_pass = Wkiti::Security::hash_password("user_password_123");
// Output: 64-character Hexadecimal String (e.g. "a591a6d40bf420404a011733cfb7b190d62c...")

Verification during Login

code<>

Wkiti::User user = Wkiti::User::find_by_email(req.body_email);
std::string attempt_hash = Wkiti::Security::hash_password(req.body_password);

if (user.password == attempt_hash) {
    // Login successful
}

2. Input Sanitization & Escaping

Wkiti provides utility methods to neutralize common web attack vectors:
Preventing XSS (Cross-Site Scripting)
Wkiti::Security::escape_html() converts dangerous HTML characters into HTML entities:

code<>

std::string clean_user_input = Wkiti::Security::escape_html("<script>alert('hack');</script>");
// Output: "&lt;script&gt;alert(&#39;hack&#39;);&lt;/script&gt;"
Preventing SQL Injection
Wkiti::Security::escape_sql() sanitizes raw queries, though using Prepared Statements (Phase 1.0) is the recommended primary defense.

3. Cookie Management & Security
Wkiti sets security flags automatically on all cookies:

code<>

res.set_cookie("session_id", "secure_token_998877");
Generates HTTP Header: Set-Cookie: session_id=secure_token_998877; Path=/; HttpOnly

HttpOnly: Prevents client-side JavaScript from accessing session tokens (mitigating XSS session hijacking).

Path=/: Ensures the cookie is valid across all application routes.

4. HTTPS / TLS 1.3 Configuration

Wkiti uses OpenSSL to wrap low-level Windows Sockets (SOCKET) inside encrypted SSL streams (SSL*).
Generating SSL Certificates
Generate a self-signed certificate for local testing:

code<>
Powershell
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365 -nodes -subj "/CN=localhost"

Enabling HTTPS in Code

code<>

int main() {
    Wkiti::Server app;

    // Load PEM files
    app.use_https("cert.pem", "key.pem");

    // Enable Strict-Transport-Security (HSTS) via Middleware
    app.use([](const auto& req, auto& res) {
        res.headers["Strict-Transport-Security"] = "max-age=31536000; includeSubDomains";
        return true;
    });

    app.listen(8443); // Runs encrypted on port 8443