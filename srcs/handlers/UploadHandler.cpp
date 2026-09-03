#include "../../includes/UploadHandler.hpp"
#include "../../includes/ErrorBuilder.hpp"
#include "../../includes/HttpResponse.hpp"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

UploadHandler::UploadHandler() {}
UploadHandler::~UploadHandler() {}

bool UploadHandler::handle(const HttpRequest& req, const LocationConfig& loc, Client& client)
{
	std::string uri = req.getUri();

	// nome do arquivo = último pedaço da URI (ex.: /uploads/foto.png -> foto.png)
	size_t lastSlash = uri.find_last_of('/');
	std::string filename = (lastSlash == std::string::npos) ? uri : uri.substr(lastSlash + 1);

	if (filename.empty()) {
		std::cout << "[UPLOAD] Sem nome de arquivo na URI: " << uri << std::endl;
		HttpResponse res;
		res.status_code = 400;
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Type"] = "text/html; charset=UTF-8";
		res.body = "<html><body><center><h1>400 Bad Request</h1>"
			"<p>No filename in URI</p></center></body></html>";
		client.setResponse(res.serialize());
		client.setState(Client::WRITING);
		return true;
	}

	std::string filePath;
	if (!loc.getUploadDir().empty()) {
		// upload_dir explícito na location: pasta de armazenamento
		// dedicada, separada de onde o GET serve os arquivos -- só o
		// nome do arquivo importa aqui, não o resto do caminho da URI.
		std::string dir = loc.getUploadDir();
		if (dir[dir.size() - 1] == '/')
			dir.erase(dir.size() - 1);
		filePath = dir + "/" + filename;
	} else {
		// sem upload_dir: usa root + URI inteira, exatamente como
		// StaticHandler/DeleteHandler resolvem o mesmo caminho -- assim
		// o arquivo aparece onde um GET/DELETE subsequente vai procurar.
		std::string root = loc.getRoot().empty() ? "./www" : loc.getRoot();
		if (root[root.size() - 1] == '/')
			root.erase(root.size() - 1);
		filePath = root + uri; // uri já começa com "/"
	}

	// error_page 500 ...; resolvido uma vez -- usado nos dois pontos de
	// falha abaixo em vez de sempre cair na página genérica do ErrorBuilder.
	std::string errorPage500 = "";
	const std::map<int, std::string>& errorPages500 = loc.getErrorPages();
	std::map<int, std::string>::const_iterator it500 = errorPages500.find(500);
	if (it500 != errorPages500.end())
		errorPage500 = ErrorBuilder::resolvePagePath(loc.getRoot(), it500->second);

	// confere se o diretório pai existe de verdade antes de tentar escrever
	size_t dirEnd = filePath.find_last_of('/');
	std::string parentDir = (dirEnd == std::string::npos) ? "." : filePath.substr(0, dirEnd);
	struct stat dirStat;
	if (stat(parentDir.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode)) {
		std::cout << "[UPLOAD] Diretório de destino não existe: " << parentDir << std::endl;
		client.setResponse(ErrorBuilder::build(500, errorPage500));
		client.setState(Client::WRITING);
		return true;
	}

	std::ofstream outFile(filePath.c_str(), std::ios::binary | std::ios::trunc);
	if (!outFile.is_open()) {
		std::cout << "[UPLOAD] Não foi possível abrir para escrita: " << filePath << std::endl;
		client.setResponse(ErrorBuilder::build(500, errorPage500));
		client.setState(Client::WRITING);
		return true;
	}

	const std::string& body = req.getBody();
	outFile.write(body.data(), static_cast<std::streamsize>(body.size()));
	outFile.close();

	std::cout << "[UPLOAD] Arquivo salvo: " << filePath << " (" << body.size() << " bytes)" << std::endl;

	HttpResponse res;
	res.status_code = 201;
	res.headers["Content-Type"] = "text/html";
		res.headers["Content-Type"] = "text/html; charset=UTF-8";
	res.headers["Location"] = uri;
	res.body = "<html><body><center><h1>201 Created</h1><p>Uploaded to "
		+ uri + "</p></center></body></html>";
	client.setResponse(res.serialize());
	client.setState(Client::WRITING);
	return true;
}