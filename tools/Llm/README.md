# Llm — a CPU inference engine built on Traktor

A standalone example: a GGUF language-model runner and a ChatGPT-style chat
window, written entirely against Traktor's own `Core` and `Ui`. No third-party
code, no GPU. It is not part of the engine — it links against it the way any
external tool would, through `ExternalDependency` on the main solution.

## What it supports

- **Container:** GGUF v2 and v3, memory mapped, weights decoded on the fly.
- **Quantization:** F32, F16, Q4_0, Q8_0, Q4_K, Q6_K — enough for the common
  `Q4_K_M`, `Q4_K_S` and `Q8_0` downloads. Anything else is refused by name.
- **Architectures:** `llama` and `mistral` (normal rotary) and `qwen2` (NeoX
  rotary). Others are refused rather than guessed at, because a wrong rotary
  type still reads fluently — it just stops tracking word order.
- **Tokenizers:** SentencePiece with byte fallback, and byte-level BPE with the
  GPT-2, Llama 3 and Qwen 2 pre-tokenizer patterns.
- **Chat markup:** ChatML, Llama 3 and Llama 2, recognized from the markers the
  model's Jinja template mentions rather than from its name.
- **Rotary scaling:** linear, and the per-channel `rope_freqs.weight` form that
  Llama 3.1 and later use. YaRN and LongRoPE are detected and warned about.
- **Sampling:** temperature, top-k, top-p and a repetition penalty, or greedy.

## Building

```
./build-projects-make-linux.sh          # generates tools/Llm/build/linux
cd build/linux/Llm     && make ReleaseShared
cd build/linux/Llm.App && make ReleaseShared
```

It links against the Traktor shared libraries aggregated in
`bin/latest/<platform>/releaseshared`, so the engine has to have been built
first. Its own products are aggregated next to them, which is why `run.sh`
needs no library path of its own.

## Running

```
./run.sh                             # open the chat window, then Open model...
./run.sh model.gguf                  # open the chat window with a model loaded
./run.sh -context=8192 model.gguf    # more context, at the cost of cache memory
./run.sh -selftest model.gguf        # bring-up report, no window
```

`-context=N` sets how many positions to allocate, defaulting to 4096. It is
clamped to what the model was trained for, quietly when that is just the
default landing above a small model and with a warning when you asked for more
than the model can do. The key/value cache is reported before it is allocated,
because it is the largest thing here and grows linearly with the context: a 3B
model with 36 layers wants 288 MiB at 4096 and 2.25 GiB at its full 32k.

`-selftest [-tokens=N]` is the first thing to reach for with a new model. It
prints the hyperparameters, the rotary scaling in force, every tensor's shape
and storage type with its first decoded weights, a tokenizer round trip, a
summary of the logits, and a short greedy generation with its rates.

The **logits line** is the one to diff — across a code change, or against a
reference implementation. It moves on any difference in the forward pass,
including differences far too small to change the sampled token. A greedy
argmax is not a sensitive test.

## Testing without a model

`scripts/misc/make-test-gguf.py` writes synthetic GGUF files that need no
download, together with a `.expected` file giving the values every tensor must
decode to. The quantized blocks are generated field by field from the format
definition, so the expected values are an independent reading of the same
layout this engine implements.

```
python3 ../../scripts/misc/make-test-gguf.py out.gguf
python3 ../../scripts/misc/make-test-gguf.py --tokenizer bpe --pre qwen2 out.gguf
python3 ../../scripts/misc/make-test-gguf.py --rope-freqs varied out.gguf
```

## Known limits

- **Speed.** The kernels use `Vector4`, which is SSE2; Traktor's math has no
  AVX2 path and there is no int8 dot product. Expect several times less than
  llama.cpp on the same machine. An AVX2 kernel would be the single biggest
  win available.
- **No batched prefill.** A prompt is read one token at a time, so a long
  prompt costs one full forward pass per token.
- **Context defaults to 4096**, not to what the model was trained for, so long
  conversations are truncated from the front sooner than the model could
  manage. Raise it with `-context=N` if you have the memory for the cache.
- **The transcript wraps, the input does not.** `ui::Edit` is single line;
  Enter sends.
