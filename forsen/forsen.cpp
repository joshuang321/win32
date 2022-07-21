#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>
#include <iostream>

#define FORSEN 160

int main(int argc, char* argv[])
{
    for (int i=0; i<FORSEN; i++)
    {
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":1,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":2,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":3,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":4,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":5,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":6,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":7,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":8,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":9,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":10,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        system("curl -X POST https://ictwebapi.azurewebsites.net/api/votes -H \"Content-Type: application/json\" -d \"{\\\"id\\\":0,\\\"bookId\\\":11,\\\"title\\\":null,\\\"justification\\\":"
        "\\\"forsen\\\",\\\"createdAt\\\":\\\"0001-01-01T00:00:00\\\"}\"");
        Sleep(700);
    }
}