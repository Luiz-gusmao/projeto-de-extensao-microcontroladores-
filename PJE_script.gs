function doGet(e) {
  Logger.log( JSON.stringify(e) ); // Registra o objeto de evento recebido para depuração
  
  var result = 'Ok'; // Inicializa a variável de resultado
  
  if (e.parameter == 'undefined') {
    result = 'No Parameters'; // Verifica se não há parâmetros
  } else {
    var sheet_id = '1gtQV2BFT4LArK3L9kaEiwdvvSIq2sY5C2WlGJPK-WkM'; // ID da planilha
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet(); // Abre a planilha
    var lastRow = sheet.getLastRow(); // Obtém o número total de linhas
    var maxRows = 4000; // Define o limite máximo de linhas
    
    // Verifica se o limite máximo de linhas foi atingido
    if (lastRow >= maxRows) {
      // Remove a primeira linha de dados e reorganiza as linhas
      sheet.deleteRow(2); // Exclui a segunda linha (índice 2) para remover a primeira linha de dados
    }
    
    var newRow = lastRow >= maxRows ? maxRows : lastRow + 1; // Obtém a próxima linha para inserção
    
    var rowData = []; // Inicializa o array de dados para a nova linha
    var Curr_Date = new Date();
    rowData[0] = Curr_Date; // Data na coluna A
    var Curr_Time = Utilities.formatDate(Curr_Date, "America/Sao_Paulo", 'HH:mm:ss');
    rowData[1] = Curr_Time; // Hora na coluna B
    
    // Loop através dos parâmetros recebidos
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]); // Remove as aspas dos valores
      Logger.log(param + ':' + e.parameter[param]);
      
      // Verifica o parâmetro atual e adiciona o valor na posição apropriada
      switch (param) {
        case 'temperature':
          rowData[2] = value; // Temperatura na coluna C
          result = 'Temperature Written on column C';
          break;
        case 'humidity':
          rowData[3] = value; // Umidade na coluna D
          result += ' ,Humidity Written on column D';
          break;
        case 'Msensor':
          rowData[4] = value; // Umidade do solo na coluna E
          result += ' ,Soil moisture on E';
          break;
        default:
          result = "unsupported parameter"; // Parâmetro não suportado
      }
    }
    
    Logger.log(JSON.stringify(rowData)); // Registra os dados da nova linha
    
    // Obtém o intervalo para os novos dados e os define na planilha
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  
  return ContentService.createTextOutput(result); // Retorna o resultado como texto
}

// Função para remover as aspas de uma string
function stripQuotes(value) {
  return value.replace(/^["']|['"]$/g, "");
}
