## Parser de arquivo `.ini`

Inspirado neste tutorial: [*“A simple C++ INI file reader”*](https://tuttlem.github.io/2012/12/26/a-simple-c-ini-file-reader.html) — fornece um parser leve que:

- Divide o arquivo em **seções** (`[window]`, `[modelo_paths]`, etc.)
- Armazena cada `chave=valor` em um `std::map<string, map<string,string>>`
- Permite recuperar os valores com chave composta: `config["window"]["title"]`, etc.

O código do tutorial mostra exatamente a lógica usada no nosso `loadConfig`.
O projeto carrega parâmetros configuráveis de um arquivo `config.ini` com base em seções e chaves. O carregamento é feito pelo parser `.ini` próprio escrito em C++.

