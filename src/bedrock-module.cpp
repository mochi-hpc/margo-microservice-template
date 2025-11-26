/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "alpha/alpha-server.h"
#include <bedrock/AbstractComponent.hpp>

namespace tl = thallium;

class AlphaComponent : public bedrock::AbstractComponent {

    std::shared_ptr<alpha_provider> m_provider;

    public:

    AlphaComponent(const tl::engine& engine,
                   uint16_t  provider_id,
                   const std::string& config,
                   const tl::pool& pool)
    {
        margo_instance_id mid = engine.get_margo_instance();
        struct alpha_provider_args alpha_args = { 0, nullptr };
        alpha_args.pool = pool.native_handle();

        alpha_provider_t provider = nullptr;
        alpha_return_t ret = alpha_provider_register(
                mid, provider_id, config.c_str(), &alpha_args,
                (alpha_provider_t*)provider);
        if(ret != ALPHA_SUCCESS)
            throw bedrock::Exception{
                std::string{"Could not instantiate Alpha provider, alpha_provider_register returned "}
                + std::to_string(ret)};
        m_provider = std::shared_ptr<alpha_provider>{
            provider, alpha_provider_destroy};
    }

    void* getHandle() override {
        return static_cast<void*>(m_provider.get());
    }

    std::string getConfig() override {
        auto cfg = alpha_provider_get_config(m_provider.get());
        auto config = std::string{cfg};
        free(cfg);
        return config;
    }

    static std::shared_ptr<bedrock::AbstractComponent>
        Register(const bedrock::ComponentArgs& args) {
            tl::pool pool;
            auto it = args.dependencies.find("pool");
            if(it != args.dependencies.end() && !it->second.empty()) {
                pool = it->second[0]->getHandle<tl::pool>();
            }
            return std::make_shared<AlphaComponent>(
                args.engine, args.provider_id, args.config, pool);
        }

    static std::vector<bedrock::Dependency>
        GetDependencies(const bedrock::ComponentArgs& args) {
            (void)args;
            std::vector<bedrock::Dependency> dependencies{
                bedrock::Dependency{
                    /* name */ "pool",
                    /* type */ "pool",
                    /* is_required */ false,
                    /* is_array */ false,
                    /* is_updatable */ false
                }
            };
            return dependencies;
        }
};

BEDROCK_REGISTER_COMPONENT_TYPE(alpha, AlphaComponent)
