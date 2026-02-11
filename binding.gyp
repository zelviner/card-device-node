{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "addon.cc" ],

      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "."
      ],

      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],

      "libraries": [
        "<(module_root_dir)/lib/card_device.lib"
      ],

      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],

      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],

      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 0
        }
      }
    }
  ]
}