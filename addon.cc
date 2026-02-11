// #include <napi.h>
// #include "card_device.h"

// /* ===================== 工具函数 ===================== */

// static CARD_DEVICE GetDevice(const Napi::CallbackInfo& info, size_t index = 0) {
//     if (info.Length() <= index || !info[index].IsExternal()) {
//         Napi::TypeError::New(info.Env(), "Invalid device handle")
//             .ThrowAsJavaScriptException();
//         return nullptr;
//     }

//     return static_cast<CARD_DEVICE>(
//         info[index].As<Napi::External<void>>().Data()
//     );
// }

// /* ===================== create() ===================== */
// Napi::Value Create(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     CARD_DEVICE dev = APP_Create();
//     if (!dev) {
//         Napi::Error::New(env, "APP_Create failed")
//             .ThrowAsJavaScriptException();
//         return env.Null();
//     }

//     // 使用 External 保存 native 句柄，GC 时自动释放
//     return Napi::External<void>::New(
//         env,
//         dev,
//         [](Napi::Env /*env*/, void* data) {
//             APP_Destroy(static_cast<CARD_DEVICE>(data));
//         }
//     );
// }

// /* ===================== initialize(dev, type) ===================== */
// Napi::Value Initialize(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     if (info.Length() < 2 || !info[1].IsNumber()) {
//         Napi::TypeError::New(env, "initialize(dev, readerType)")
//             .ThrowAsJavaScriptException();
//         return env.Null();
//     }

//     CARD_DEVICE dev = GetDevice(info, 0);
//     if (!dev) return env.Null();

//     int readerType = info[1].As<Napi::Number>().Int32Value();

//     const char* readers[16] = {0};
//     int readerCount = 16;

//     BOOL ok = APP_Initialize(dev, readerType, readers, &readerCount);
//     if (!ok) {
//         Napi::Object ret = Napi::Object::New(env);
//         ret.Set("ok", false);
//         return ret;
//     }

//     Napi::Array arr = Napi::Array::New(env, readerCount);
//     for (int i = 0; i < readerCount; ++i) {
//         arr.Set(i, Napi::String::New(env, readers[i]));
//     }

//     Napi::Object ret = Napi::Object::New(env);
//     ret.Set("ok", true);
//     ret.Set("readers", arr);
//     return ret;
// }

// /* ===================== cardProtocol(dev, proto) ===================== */
// Napi::Value CardProtocol(const Napi::CallbackInfo& info) {
//     if (info.Length() < 2 || !info[1].IsNumber()) {
//         Napi::TypeError::New(info.Env(), "cardProtocol(dev, proto)")
//             .ThrowAsJavaScriptException();
//         return info.Env().Null();
//     }

//     CARD_DEVICE dev = GetDevice(info, 0);
//     if (!dev) return info.Env().Null();

//     int proto = info[1].As<Napi::Number>().Int32Value();

//     return Napi::Boolean::New(
//         info.Env(),
//         APP_CardProtocol(dev, proto)
//     );
// }

// /* ===================== cardReader(dev, index) ===================== */
// Napi::Value CardReader(const Napi::CallbackInfo& info) {
//     if (info.Length() < 2 || !info[1].IsNumber()) {
//         Napi::TypeError::New(info.Env(), "cardReader(dev, index)")
//             .ThrowAsJavaScriptException();
//         return info.Env().Null();
//     }

//     CARD_DEVICE dev = GetDevice(info, 0);
//     if (!dev) return info.Env().Null();

//     size_t index = info[1].As<Napi::Number>().Uint32Value();

//     return Napi::Boolean::New(
//         info.Env(),
//         APP_CardReader(dev, index)
//     );
// }

// /* ===================== cardCallback(dev, callback) ===================== */
// struct CardCallbackContext {
//     Napi::ThreadSafeFunction tsfn;
// };

// static void NativeCardCallback(
//     const char *data,
//     int len,
//     void *user
// ) {
//     auto *ctx = static_cast<CardCallbackContext *>(user);
//     if (!ctx) return;

//     // 把数据拷贝出来（一定要拷贝）
//     std::string payload(data, len);

//     napi_status status = ctx->tsfn.BlockingCall(
//         new std::string(std::move(payload)),
//         [](Napi::Env env,
//            Napi::Function jsCallback,
//            std::string *value) {

//             // 转成 Buffer 传给 JS
//             Napi::Buffer<char> buf =
//                 Napi::Buffer<char>::Copy(
//                     env,
//                     value->data(),
//                     value->size()
//                 );

//             jsCallback.Call({ buf });
//             delete value;
//         }
//     );

//     if (status != napi_ok) {
//         // 这里一般不用 throw，静默失败即可
//     }
// }

// Napi::Value CardCallbackRegister(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     if (info.Length() < 2 || !info[1].IsFunction()) {
//         Napi::TypeError::New(
//             env,
//             "cardCallback(dev, callback)"
//         ).ThrowAsJavaScriptException();
//         return env.Null();
//     }

//     CARD_DEVICE dev = GetDevice(info, 0);
//     if (!dev) return env.Null();

//     Napi::Function jsCallback = info[1].As<Napi::Function>();

//     auto *ctx = new CardCallbackContext();

//     ctx->tsfn = Napi::ThreadSafeFunction::New(
//         env,
//         jsCallback,           // JS 回调
//         "CardCallback",       // 资源名
//         0,                    // queue size（0 = unlimited）
//         1                     // 单线程使用
//     );

//     BOOL ok = APP_CardCallback(
//         dev,
//         NativeCardCallback,
//         ctx
//     );

//     if (!ok) {
//         ctx->tsfn.Release();
//         delete ctx;
//         return Napi::Boolean::New(env, false);
//     }

//     return Napi::Boolean::New(env, true);
// }

// /* ===================== resetCard(dev, cold) ===================== */
// Napi::Value ResetCard(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     if (info.Length() < 2 || !info[1].IsBoolean()) {
//         Napi::TypeError::New(env, "resetCard(dev, cold)")
//             .ThrowAsJavaScriptException();
//         return env.Null();
//     }

//     CARD_DEVICE dev = GetDevice(info, 0);
//     if (!dev) return env.Null();

//     BOOL cold = info[1].As<Napi::Boolean>().Value();

//     char atr[256] = {0};
//     BOOL ok = APP_ResetCardReader(dev, cold, atr, sizeof(atr));

//     if (!ok) return env.Null();
//     return Napi::String::New(env, atr);
// }

// /* ===================== persoData(dev, prdFile, hasDs) ===================== */
// Napi::Value PersoData(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     if (info.Length() < 3 ||
//         !info[1].IsString() ||
//         !info[2].IsBoolean()) {
//         Napi::TypeError::New(
//             env,
//             "persoData(dev, prdFile, hasDs)"
//         ).ThrowAsJavaScriptException();
//         return env.Null();
//     }

//     CARD_DEVICE dev = GetDevice(info, 0);
//     if (!dev) return env.Null();

//     std::string prdFile = info[1].As<Napi::String>().Utf8Value();
//     BOOL hasDs = info[2].As<Napi::Boolean>().Value();

//     BOOL ok = APP_PersoData(
//         dev,
//         prdFile.c_str(),
//         hasDs
//     );

//     return Napi::Boolean::New(env, ok);
// }

// /* ===================== run(dev, scriptFile, convert) ===================== */
// Napi::Value Run(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     if (info.Length() < 3 ||
//         !info[1].IsString() ||
//         !info[2].IsBoolean()) {
//         Napi::TypeError::New(
//             env,
//             "run(dev, scriptFile, convert)"
//         ).ThrowAsJavaScriptException();
//         return env.Null();
//     }

//     CARD_DEVICE dev = GetDevice(info, 0);
//     if (!dev) return env.Null();

//     std::string scriptFile = info[1].As<Napi::String>().Utf8Value();
//     BOOL convert = info[2].As<Napi::Boolean>().Value();

//     BOOL ok = APP_Run(
//         dev,
//         scriptFile.c_str(),
//         convert
//     );

//     return Napi::Boolean::New(env, ok);
// }

// /* ===================== getLastError(dev) ===================== */
// Napi::Value GetLastError(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     CARD_DEVICE dev = GetDevice(info, 0);
//     if (!dev) return env.Null();

//     char err[512] = {0};
//     APP_GetLastError(dev, err, sizeof(err));

//     return Napi::String::New(env, err);
// }

// /* ===================== 模块导出 ===================== */
// Napi::Object Init(Napi::Env env, Napi::Object exports) {
//     exports.Set("create",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return Create(info);
//         })
//     );

//     exports.Set("initialize",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return Initialize(info);
//         })
//     );

//     exports.Set("cardProtocol",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return CardProtocol(info);
//         })
//     );

//     exports.Set("cardReader",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return CardReader(info);
//         })
//     );

//     exports.Set("cardCallback",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return CardCallbackRegister(info);
//         })
//     );

//     exports.Set("resetCard",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return ResetCard(info);
//         })
//     );

//     exports.Set("persoData",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return PersoData(info);
//         })
//     );

//     exports.Set("run",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return Run(info);
//         })
//     );

//     exports.Set("getLastError",
//         Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
//             return GetLastError(info);
//         })
//     );

//     return exports;
// }

// NODE_API_MODULE(addon, Init)

#include <napi.h>
#include "card_device.h"
#include <string>

/* ============================================================
 * Device / Callback 封装
 * ============================================================ */

struct CardCallbackContext {
    Napi::ThreadSafeFunction tsfn;
};

struct DeviceWrapper {
    CARD_DEVICE dev = nullptr;
    CardCallbackContext* cb = nullptr;
};

/* ============================================================
 * 工具函数
 * ============================================================ */

static DeviceWrapper* GetWrapper(const Napi::CallbackInfo& info, size_t index = 0) {
    if (info.Length() <= index || !info[index].IsExternal()) {
        Napi::TypeError::New(info.Env(), "Invalid device handle")
            .ThrowAsJavaScriptException();
        return nullptr;
    }

    return static_cast<DeviceWrapper*>(
        info[index].As<Napi::External<void>>().Data()
    );
}

/* ============================================================
 * Native → JS 回调
 * ============================================================ */

static void NativeCardCallback(
    const char* data,
    int len,
    void* user
) {
    auto* ctx = static_cast<CardCallbackContext*>(user);
    if (!ctx) return;

    // 必须拷贝
    std::string payload(data, len);

    ctx->tsfn.NonBlockingCall(
        new std::string(std::move(payload)),
        [](Napi::Env env,
           Napi::Function jsCallback,
           std::string* value) {

            Napi::Buffer<char> buf =
                Napi::Buffer<char>::Copy(
                    env,
                    value->data(),
                    value->size()
                );

            jsCallback.Call({ buf });
            delete value;
        }
    );
}

/* ============================================================
 * create()
 * ============================================================ */

Napi::Value Create(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    CARD_DEVICE dev = APP_Create();
    if (!dev) {
        Napi::Error::New(env, "APP_Create failed")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* wrapper = new DeviceWrapper();
    wrapper->dev = dev;

    return Napi::External<void>::New(
        env,
        wrapper,
        [](Napi::Env /*env*/, void* data) {
            auto* w = static_cast<DeviceWrapper*>(data);

            if (w->cb) {
                w->cb->tsfn.Release();
                delete w->cb;
                w->cb = nullptr;
            }

            if (w->dev) {
                APP_Destroy(w->dev);
                w->dev = nullptr;
            }

            delete w;
        }
    );
}

/* ============================================================
 * initialize(dev, readerType)
 * ============================================================ */

Napi::Value Initialize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "initialize(dev, readerType)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return env.Null();

    int readerType = info[1].As<Napi::Number>().Int32Value();

    const char* readers[16] = {0};
    int readerCount = 16;

    BOOL ok = APP_Initialize(w->dev, readerType, readers, &readerCount);

    Napi::Object ret = Napi::Object::New(env);
    ret.Set("ok", ok);

    if (ok) {
        Napi::Array arr = Napi::Array::New(env, readerCount);
        for (int i = 0; i < readerCount; ++i) {
            arr.Set(i, Napi::String::New(env, readers[i]));
        }
        ret.Set("readers", arr);
    }

    return ret;
}

/* ============================================================
 * cardProtocol(dev, proto)
 * ============================================================ */

Napi::Value CardProtocol(const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "cardProtocol(dev, proto)")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return info.Env().Null();

    int proto = info[1].As<Napi::Number>().Int32Value();
    return Napi::Boolean::New(
        info.Env(),
        APP_CardProtocol(w->dev, proto)
    );
}

/* ============================================================
 * cardReader(dev, index)
 * ============================================================ */

Napi::Value CardReader(const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[1].IsNumber()) {
        Napi::TypeError::New(info.Env(), "cardReader(dev, index)")
            .ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return info.Env().Null();

    size_t index = info[1].As<Napi::Number>().Uint32Value();
    return Napi::Boolean::New(
        info.Env(),
        APP_CardReader(w->dev, index)
    );
}

/* ============================================================
 * resetCard(dev, cold)
 * ============================================================ */

Napi::Value ResetCard(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[1].IsBoolean()) {
        Napi::TypeError::New(env, "resetCard(dev, cold)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return env.Null();

    BOOL cold = info[1].As<Napi::Boolean>().Value();

    char atr[256] = {0};
    BOOL ok = APP_ResetCardReader(w->dev, cold, atr, sizeof(atr));
    if (!ok) return env.Null();

    return Napi::String::New(env, atr);
}

/* ============================================================
 * persoData(dev, prdFile, hasDs)
 * ============================================================ */

Napi::Value PersoData(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 3 ||
        !info[1].IsString() ||
        !info[2].IsBoolean()) {
        Napi::TypeError::New(env, "persoData(dev, prdFile, hasDs)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return env.Null();

    std::string prdFile = info[1].As<Napi::String>().Utf8Value();
    BOOL hasDs = info[2].As<Napi::Boolean>().Value();

    return Napi::Boolean::New(
        env,
        APP_PersoData(w->dev, prdFile.c_str(), hasDs)
    );
}

/* ============================================================
 * runFile(dev, scriptFile, convert)
 * ============================================================ */

Napi::Value RunFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 3 ||
        !info[1].IsString() ||
        !info[2].IsBoolean()) {
        Napi::TypeError::New(env, "runFile(dev, scriptFile, convert)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return env.Null();

    std::string scriptFile = info[1].As<Napi::String>().Utf8Value();
    BOOL convert = info[2].As<Napi::Boolean>().Value();

    return Napi::Boolean::New(
        env,
        APP_RunFile(w->dev, scriptFile.c_str(), convert)
    );
}

/* ============================================================
 * run(dev, scriptCode, convert)
 * ============================================================ */

Napi::Value Run(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 3 ||
        !info[1].IsString() ||
        !info[2].IsBoolean()) {
        Napi::TypeError::New(env, "run(dev, scriptCode, convert)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return env.Null();

    std::string scriptFile = info[1].As<Napi::String>().Utf8Value();
    BOOL convert = info[2].As<Napi::Boolean>().Value();

    return Napi::Boolean::New(
        env,
        APP_Run(w->dev, scriptFile.c_str(), convert)
    );
}

/* ============================================================
 * cardCallback(dev, callback)
 * ============================================================ */

Napi::Value CardCallbackRegister(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[1].IsFunction()) {
        Napi::TypeError::New(env, "cardCallback(dev, callback)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return env.Null();

    // 防二次注册
    if (w->cb) {
        w->cb->tsfn.Release();
        delete w->cb;
        w->cb = nullptr;
    }

    auto* ctx = new CardCallbackContext();
    ctx->tsfn = Napi::ThreadSafeFunction::New(
        env,
        info[1].As<Napi::Function>(),
        "CardCallback",
        0,
        1
    );

    BOOL ok = APP_CardCallback(
        w->dev,
        NativeCardCallback,
        ctx
    );

    if (!ok) {
        ctx->tsfn.Release();
        delete ctx;
        return Napi::Boolean::New(env, false);
    }

    w->cb = ctx;
    return Napi::Boolean::New(env, true);
}

/* ============================================================
 * getLastError(dev)
 * ============================================================ */

Napi::Value GetLastError(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    auto* w = GetWrapper(info, 0);
    if (!w || !w->dev) return env.Null();

    char err[512] = {0};
    APP_GetLastError(w->dev, err, sizeof(err));
    return Napi::String::New(env, err);
}

/* ============================================================
 * 模块导出
 * ============================================================ */
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("create",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return Create(info);
        })
    );

    exports.Set("initialize",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return Initialize(info);
        })
    );

    exports.Set("cardProtocol",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return CardProtocol(info);
        })
    );

    exports.Set("cardReader",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return CardReader(info);
        })
    );

    exports.Set("cardCallback",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return CardCallbackRegister(info);
        })
    );

    exports.Set("resetCard",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return ResetCard(info);
        })
    );

    exports.Set("persoData",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return PersoData(info);
        })
    );

    exports.Set("runFile",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return RunFile(info);
        })
    );

    exports.Set("run",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return Run(info);
        })
    );

    exports.Set("getLastError",
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
            return GetLastError(info);
        })
    );

    return exports;
}

NODE_API_MODULE(addon, Init)
