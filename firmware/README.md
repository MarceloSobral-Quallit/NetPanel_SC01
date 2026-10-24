# SC01_BOOTSTRAP

Primeiro firmware da SC01: conecta ao Wi-Fi e executa OTA HTTPS em duas partições de aplicativo, com rollback habilitado.

O exemplo inicial requer uma URL HTTPS de firmware e as credenciais Wi-Fi definidas localmente por `idf.py menuconfig`; esses valores nunca devem ser versionados.

Antes de gravar, execute `tools/backup-flash.ps1`. O backup original atual está em `backups/` e não é enviado ao Git.
