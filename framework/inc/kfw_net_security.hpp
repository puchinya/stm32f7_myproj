
#pragma once
#ifndef KFW_NET_SECUIRTY_HPP
#define KFW_NET_SECUIRTY_HPP

#include <kfw_net.hpp>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

namespace kfw { namespace net {

class X509Certificate;
class TlsStream;

class TlsStream final : private kfw::io::Stream
{
public:
	TlsStream() : m_is_created(false), m_is_opened_session(false), m_base_stream(nullptr) {}
	~TlsStream() override {
		dispose();
	}
	ret_t create(SocketStream *base_stream);
	void dispose();

	void flush() override;
	void close() override;
	RetVal<uint32_t> read(void *buf, uint32_t size) override;
	RetVal<uint32_t> write(const void *buf, uint32_t size) override;

	ret_t authenticate_as_client(const ConstStringRef &target_host,
			X509Certificate *ca_cert);
	ret_t authenticate_as_client(const ConstStringRef &target_host,
			X509Certificate *ca_cert,
			X509Certificate *client_cert
			);
public:

	static int ssl_send(void *ctx, const unsigned char *buf, size_t len);
	static int ssl_recv(void *ctx, unsigned char *buf, size_t len);
	static int ssl_verify(void *ctx, mbedtls_x509_crt *crt, int depth,
							 uint32_t *flags);
private:
	bool m_is_created;
	bool m_is_opened_session;
	SocketStream *m_base_stream;
	mbedtls_entropy_context	m_entropy;
	mbedtls_ctr_drbg_context m_ctr_drbg;
	mbedtls_ssl_context m_ssl;
	mbedtls_ssl_config m_ssl_conf;

};

class X509Certificate final : private NonCopyable
{
	friend class TlsStream;
public:
	X509Certificate() : m_is_created(false) {}
	~X509Certificate() {
		dispose();
	}

	// PEM形式から生成します。
	ret_t create(const void *data, size_t data_size);
	void dispose();

	bool is_created() const {
		return m_is_created;
	}

	// For TLS API
	mbedtls_x509_crt *get_handle() {
		return &m_cert;
	}

public:
	bool m_is_created;
	mbedtls_x509_crt m_cert;
};

}}

#endif
