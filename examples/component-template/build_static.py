import textwrap
import mimetypes
import os.path
import os
import re

Import("env")
Import("projenv")


header_foreword = """
    #pragma once
    #include <Husarnet.h>
    #include <Arduino.h>
    
    #if __has_include(<WebServer.h>)
    #define USE_WEBSERVER 1
    #include <WebServer.h>
    #endif

    #if __has_include(<ESPAsyncWebServer.h>)
    #define USE_ASYNCWEBSERVER 1
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #endif

"""
header_template = """
    extern const char {variable_name}[{content_length}] PROGMEM;
    extern const size_t {variable_name}_LEN;
    extern const char {variable_name}_MIME[] PROGMEM;

"""
header_afterword = """
    #ifdef USE_WEBSERVER
    void registerStaticWebServerHandlers(WebServer *server);
    #endif

    #ifdef USE_ASYNCWEBSERVER
    void registerStaticAsyncWebServerHandlers(AsyncWebServer *server);
    void registerDynamicAsyncWebServerHandlers(AsyncWebServer *server, AwsTemplateProcessor callback=nullptr);
    #endif
"""

source_foreword = """
    #include "static_data.hpp"

"""
source_template = """
    const char {variable_name}[{content_length}] PROGMEM = {{
    {hex_content}
    }};
    const size_t {variable_name}_LEN = {content_length};
    const char {variable_name}_MIME[] = "{content_mime}";

"""
source_afterword = """

"""

registration_foreword = """
    void registerStaticWebServerHandlers(WebServer *server) {
"""
registration_template = """
    server->on("{relative_file_path}", [=](){{
        server->send_P(200, {variable_name}_MIME, {variable_name}, {variable_name}_LEN);
    }});
"""
registration_afterword = """
    }

"""

async_registration_foreword = """
    void registerStaticAsyncWebServerHandlers(AsyncWebServer *server) {
"""
async_registration_template = """
    server->on("{relative_file_path}", HTTP_GET, [=](AsyncWebServerRequest *request) {{
        request->send_P(200, {variable_name}_MIME, (const uint8_t*){variable_name}, {variable_name}_LEN);
    }});
"""
async_registration_afterword = """
    }
    
"""

async_dynamic_registration_foreword = """
    void registerDynamicAsyncWebServerHandlers(AsyncWebServer *server, AwsTemplateProcessor callback) {
"""
async_dynamic_registration_template = """
    server->on("{relative_file_path}", HTTP_GET, [=](AsyncWebServerRequest *request) {{
        request->send_P(200, {variable_name}_MIME, (const uint8_t*){variable_name}, {variable_name}_LEN, callback);
    }});
"""
async_dynamic_registration_afterword = """
    }

"""


filename_re = re.compile("[^A-Z0-9_]+")


def prepare_templates():
    global header_foreword, header_template, header_afterword
    global source_foreword, source_template, source_afterword
    global registration_foreword, registration_template, registration_afterword
    global async_registration_foreword, async_registration_template, async_registration_afterword
    global async_dynamic_registration_foreword, async_dynamic_registration_template, async_dynamic_registration_afterword

    def prepare(input):
        return textwrap.dedent(input.lstrip("\n"))

    header_foreword = prepare(header_foreword)
    header_template = prepare(header_template)
    header_afterword = prepare(header_afterword)

    source_foreword = prepare(source_foreword)
    source_template = prepare(source_template)
    source_afterword = prepare(source_afterword)

    registration_foreword = prepare(registration_foreword)
    registration_template = registration_template.lstrip("\n")
    registration_afterword = prepare(registration_afterword)

    async_registration_foreword = prepare(async_registration_foreword)
    async_registration_template = async_registration_template.lstrip("\n")
    async_registration_afterword = prepare(async_registration_afterword)

    async_dynamic_registration_foreword = prepare(async_dynamic_registration_foreword)
    async_dynamic_registration_template = async_dynamic_registration_template.lstrip(
        "\n"
    )
    async_dynamic_registration_afterword = prepare(async_dynamic_registration_afterword)


def build_files():
    with open(os.path.join("src", "static_data.cpp"), "w") as source_file, open(
        os.path.join("src", "static_data.hpp"), "w"
    ) as header_file:
        header_file.write(header_foreword)
        source_file.write(source_foreword)

        def gather_files(path):
            file_library = []

            for root, dirs, files in os.walk(path):
                for name in files:
                    file_path = os.path.abspath(os.path.join(root, name))
                    relative_file_path = file_path[len(path) :]

                    with open(file_path, "rb") as f:
                        content = f.read()

                    variable_name = "STATIC_" + filename_re.sub(
                        "_", relative_file_path[1:].upper()
                    )

                    hex_content = ", ".join("0x{:02X}".format(b) for b in content)
                    hex_content = "\n".join(
                        textwrap.wrap(
                            hex_content, initial_indent="    ", subsequent_indent="    "
                        )
                    )

                    content_length = len(content)

                    content_mime = mimetypes.guess_type(file_path, strict=False)[0]
                    if content_mime is None:
                        content_mime = "text/plain"

                    kwargs = {
                        "variable_name": variable_name,
                        "hex_content": hex_content,
                        "content_length": content_length,
                        "content_mime": content_mime,
                    }

                    header_file.write(header_template.format(**kwargs))
                    source_file.write(source_template.format(**kwargs))

                    file_library.append((relative_file_path, variable_name))

            return file_library

        def write_registration(foreword, template, afterword, library):
            source_file.write(foreword)

            for relative_file_path, variable_name in library:
                source_file.write(
                    template.format(
                        relative_file_path=relative_file_path,
                        variable_name=variable_name,
                    )
                )

            source_file.write(afterword)

        project_path = os.path.abspath("")
        static_path = os.path.join(project_path, "static")
        dynamic_path = os.path.join(project_path, "dynamic")

        static_files = gather_files(static_path)
        dynamic_files = gather_files(dynamic_path)

        write_registration(
            registration_foreword,
            registration_template,
            registration_afterword,
            static_files,
        )
        write_registration(
            async_registration_foreword,
            async_registration_template,
            async_registration_afterword,
            static_files,
        )

        write_registration(
            async_dynamic_registration_foreword,
            async_dynamic_registration_template,
            async_dynamic_registration_afterword,
            dynamic_files,
        )

        header_file.write(header_afterword)
        source_file.write(source_afterword)

    print("Regenerated static data")


if __name__ == "SCons.Script":
    prepare_templates()
    build_files()
else:
    print("Skipping static data build")