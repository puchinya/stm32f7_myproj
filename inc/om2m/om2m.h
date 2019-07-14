
typedef struct om2mIUnknwon_vtbl {

};

typedef struct om2m_unknown_t {

} om2m_unknown_t;

typedef struct om2m_cse_cfg_t {
	const char *cse_base_address;
	const char *cse_base_context;
	const char *cse_base_id;
	const char *cse_base_name;
	const char *admin_acp_name;
	const char *admin_originator;
} om2m_cse_cfg_t;

om2m_err_t om2m_ae_create();
om2m_err_t om2m_ae_release();

typedef struct {

} OM2MController;
