/*******************************************************************************
* MIT License
*
* Copyright (c) 2024 Curtis McCoy
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

// Simple entry point for loaidng wasm, providing a basic output method, and
//    displaying spec results.
// TODO: make mode where it prints via HTML elements

window.onload = async () => {

  const utf8_decoder = new TextDecoder("utf-8");
  const html_body = document.getElementById("console");

  let wasm = null;

  let get_str = (index, size) => {
    let array = new Uint8Array(wasm.exports.memory.buffer);
    array = array.subarray(index, index + size);
    return utf8_decoder.decode(array);
  };

  let imports = {

    'js_log': (str, len, color) => {
      //* // Output to HTML-based "console"
      let row = document.createElement("div");
      let text = get_str(str, len);
      let index = text.indexOf("%c");
      let fmt = 'color:#' + (color & 0x00ffffff).toString(16).padStart(6, '0');
      text = text.replace(/ /g, " ");
      if (color & 0x1000000) fmt += ';font-weight:bold';
      if (index == -1) {
        row.innerText = text;
      } else {
        let first = document.createElement("span");
        let second = document.createElement("span");
        first.innerText = text.slice(0, index);
        second.innerText = text.slice(index + 2);
        second.style = fmt;
        row.appendChild(first);
        row.appendChild(second);
      }
      html_body.appendChild(row);
      window.scrollTo(0, document.body.scrollHeight);
      /*/ // Output to js console
      if (color < 0) {
        console.log(get_str(str, len));
        return;
      }
      let fmt = 'color:#' + (color & 0x00ffffff).toString(16).padStart(6, '0');
      if (color & 0x1000000) {
        fmt += ';font-weight:bold';
      }
      console.log(get_str(str, len), fmt);
      //*/
    }

  };

  let wasm_reload = async (op) => {
    wasm = null;
    const response = await fetch("test.wasm");
    await WebAssembly.instantiateStreaming(response, { env: imports })
    .then((obj) => {
      wasm = obj.instance;
      if (!wasm) {
        console.log("Failed to load WASM");
      } else {
        op();
      }
    });
  }

  let input_line = document.getElementById("input-line");
  let input_nan = document.getElementById("input-nan");

  let line_value = () => {
    let value = input_line.value;
    return isNaN(value) ? 0 : parseInt(value);
  }

  document.getElementById("btn-clear").onclick = (e) => {
    html_body.replaceChildren();
  }

  document.getElementById("btn-retry").onclick = (e) => {
    wasm_reload(() => {
      wasm.exports.spec_main(1, line_value());
    });
  }

  document.getElementById("btn-retry-v").onclick = (e) => {
    wasm_reload(() => {
      wasm.exports.spec_main(2, line_value());
    });
  }

  document.getElementById("btn-retry-vf").onclick = (e) => {
    wasm_reload(() => {
      wasm.exports.spec_main(3, line_value());
    });
  }

  input_line.onkeyup = (e) => {
    input_nan.hidden =
          input_line.value == ""
      || !isNaN(parseInt(input_line.value));
  }

  input_line.onkeydown = (e) => {
    if (e.key == "Enter") {
      input_line.select();
      wasm_reload(() => {
        wasm.exports.spec_main(2, line_value());
      });
    }
  }

  await wasm_reload(() => {
    wasm.exports.spec_main(2, 0);
  });

  console.log('done');
}
