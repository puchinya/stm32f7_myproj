
#include "kfw_net_security.hpp"

namespace kfw { namespace net {

extern "C" {
	static int ssl_send_clink(void *ctx, const unsigned char *buf, size_t len) {
		return TlsStream::ssl_send(ctx, buf, len);
	}
	static int ssl_recv_clink(void *ctx, unsigned char *buf, size_t len) {
		return TlsStream::ssl_recv(ctx, buf, len);
	}
	static int ssl_verify_clink(void *ctx, mbedtls_x509_crt *crt, int depth,
								 uint32_t *flags)
	{
		return TlsStream::ssl_verify(ctx, crt, depth, flags);
	}
};

ret_t TlsStream::create(SocketStream *base_stream)
{
	if(m_is_created) {
		return kEInvalidOperation;
	}

	m_base_stream = base_stream;

	mbedtls_entropy_init(&m_entropy);
	mbedtls_ctr_drbg_init(&m_ctr_drbg);
	mbedtls_ssl_init(&m_ssl);
	mbedtls_ssl_config_init(&m_ssl_conf);

	m_is_created = true;

	return kOk;
}

void TlsStream::dispose()
{
	if(!m_is_created) return;

	mbedtls_entropy_free(&m_entropy);
	mbedtls_ctr_drbg_free(&m_ctr_drbg);
	mbedtls_ssl_free(&m_ssl);
	mbedtls_ssl_config_free(&m_ssl_conf);

	m_base_stream = nullptr;

	m_is_created = false;
}


void TlsStream::flush()
{
	if(!m_is_opened_session) return;

	m_base_stream->flush();
}

void TlsStream::close()
{
	if(!m_is_opened_session) return;

	m_base_stream->close();
}

RetVal<uint32_t> TlsStream::read(void *buf, uint32_t size)
{
	if(!m_is_opened_session) {
		return RetVal<uint32_t>(kEInvalidOperation, 0);
	}
	auto len = mbedtls_ssl_read(&m_ssl, (uint8_t *)buf, size);
	if(len >= 0) {
		return RetVal<uint32_t>(kOk, len);
	} else {
		return RetVal<uint32_t>(kEUnknown, 0);
	}
}

RetVal<uint32_t> TlsStream::write(const void *buf, uint32_t size)
{
	if(!m_is_opened_session) {
		return RetVal<uint32_t>(kEInvalidOperation, 0);
	}
	auto len = mbedtls_ssl_write(&m_ssl, (const uint8_t *)buf, size);
	if(len >= 0) {
		return RetVal<uint32_t>(kOk, len);
	} else {
		return RetVal<uint32_t>(kEUnknown, 0);
	}
}

const char *DRBG_PERSONALIZED_STR = "Mbed TLS helloword client";

ret_t TlsStream::authenticate_as_client(const ConstStringRef &target_host,
		X509Certificate *ca_cert)
{
	ret_t r;
	int ssl_ret;

	if(target_host.get_length() > 31) {
		return kEArgument;
	}

	if(ca_cert == nullptr || !ca_cert->is_created()) {
		return kEArgument;
	}

	if(m_is_opened_session) {
		return kEInvalidOperation;
	}

	ssl_ret = mbedtls_ctr_drbg_seed(&m_ctr_drbg, mbedtls_entropy_func, &m_entropy,
			reinterpret_cast<const unsigned char *>(DRBG_PERSONALIZED_STR),
			strlen(DRBG_PERSONALIZED_STR) + 1);
	if(ssl_ret != 0) {
		return kEUnknown;
	}

	ssl_ret = mbedtls_ssl_config_defaults(&m_ssl_conf, MBEDTLS_SSL_IS_CLIENT,
	                                      MBEDTLS_SSL_TRANSPORT_STREAM,
	                                      MBEDTLS_SSL_PRESET_DEFAULT);
	if(ssl_ret != 0) {
		return kEUnknown;
	}

	mbedtls_ssl_conf_ca_chain(&m_ssl_conf, ca_cert->get_handle(), nullptr);
	mbedtls_ssl_conf_rng(&m_ssl_conf, mbedtls_ctr_drbg_random, &m_ctr_drbg);
	mbedtls_ssl_conf_authmode(&m_ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
	mbedtls_ssl_conf_verify(&m_ssl_conf, ssl_verify_clink, this);

	ssl_ret = mbedtls_ssl_setup(&m_ssl, &m_ssl_conf);
	if(ssl_ret != 0) {
		return kEUnknown;
	}

	char host_name_buf[32];
	memcpy(host_name_buf, target_host.get_data(), target_host.get_length());
	host_name_buf[target_host.get_length()] = 0;

	ssl_ret = mbedtls_ssl_set_hostname(&m_ssl, host_name_buf);
	if(ssl_ret != 0) {
		return kEUnknown;
	}

	mbedtls_ssl_set_bio(&m_ssl, static_cast<void *>(m_base_stream),
			ssl_send_clink, ssl_recv_clink, nullptr);

	// start handshake
	do {
		ssl_ret = mbedtls_ssl_handshake(&m_ssl);
	} while(ssl_ret != 0 &&
            (ssl_ret == MBEDTLS_ERR_SSL_WANT_READ ||
            		ssl_ret == MBEDTLS_ERR_SSL_WANT_WRITE));
	if(ssl_ret < 0) {
		return kEUnknown;
	}

	m_is_opened_session = true;

	return kOk;
}

int TlsStream::ssl_send(void *ctx, const unsigned char *buf, size_t len)
{
	SocketStream *s = (SocketStream *)ctx;

	auto r = s->write(buf, len);

	return r.val;
}

int TlsStream::ssl_recv(void *ctx, unsigned char *buf, size_t len)
{
	SocketStream *s = (SocketStream *)ctx;

	auto r = s->read(buf, len);

	return r.val;
}

int TlsStream::ssl_verify(void *ctx, mbedtls_x509_crt *crt, int depth,
                         uint32_t *flags)
{
	TlsStream *self = (TlsStream *)ctx;

	*flags &= ~MBEDTLS_X509_BADCERT_FUTURE & ~MBEDTLS_X509_BADCERT_EXPIRED;

	return 0;
}

//--- X509Certificate class ---
ret_t X509Certificate::create(const void *data, size_t data_size)
{
	if(data == nullptr || data_size == 0) {
		return kEArgument;
	}

	if(m_is_created) {
		return kEInvalidOperation;
	}

	// memsetで0クリアするのみ。
	mbedtls_x509_crt_init(&m_cert);

	int ssl_ret = mbedtls_x509_crt_parse(&m_cert,
			(const uint8_t *)data, data_size);
	if(ssl_ret != 0) {
		return kEUnknown;
	}

	m_is_created = true;

	return kOk;
}

void X509Certificate::dispose()
{
	if(!m_is_created) return;

	mbedtls_x509_crt_free(&m_cert);

	m_is_created = false;
}

}}
