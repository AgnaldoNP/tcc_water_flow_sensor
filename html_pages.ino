String getConfigPage(){
  String html = "";

html +="<html><head><meta name='viewport' content='width=device-width, height=device-height, initial-scale=1'><style>";
html +="#main{width: 80%;position: relative; margin: auto; }  h3{text-align: center;} input{width: 100%; margin-top: 1px; border: 1px solid #aaa; padding: 3px; font-size: }";
html +="label {color: #333; margin-top: 5px; display: inline-block;} input[type='submit'] {margin-top: 5px;}</style></head><body><div id='main'><h3>Configura&ccedil;&otilde;es</h3>";
html +="<form action='./config/set' method='post'><label>Identificador do usuario</label><input type='text' id='identifier' name='identifier' placeholder='Identificador do usuario' /><br/>";
html +="<label>Rede WiFi</label><input type='text' id='ssid' name='ssid' placeholder='Rede WiFi' /><br/><label>Senha WiFi</label><input type='text' id='pass' name='pass' placeholder='Senha WiFi' /><br/>";
html +="<input type='submit' value='Salvar' /></form></div></body></html>";

  return html;
}

