local function change_dirs()
    vim.cmd.chdir(vim.fn.expand("~/Desktop/C++/Enet_tutorial"))
end


-- Define the paths for the .bat file and executables
local compile_bat = "compile.bat"
local build_dir = "build\\" -- Use double backslashes for Windows paths
local client_exe = build_dir .. "client.exe"
local server_exe = build_dir .. "server.exe"

local function run_server()
    change_dirs()
    -- Quote the executable path for safety
    local quoted_server_exe = '"' .. server_exe .. '"'
    vim.cmd("tabnew")
    vim.cmd("term " .. quoted_server_exe)
end

local function run_client()
    change_dirs()
    -- Quote the executable path for safety
    local quoted_client_exe = '"' .. client_exe .. '"'
    vim.cmd("tabnew")
    vim.cmd("term " .. quoted_client_exe)
end

local function run_code()
    change_dirs()

    -- Run the compile.bat file and capture its output
    local compile_cmd = "cmd /c " .. compile_bat
    local compile_status = os.execute(compile_cmd)

    -- Check if the compilation was successful
    if compile_status ~= 0 then
        -- Show an error message if the compilation fails
        vim.api.nvim_err_writeln("Compilation failed. Check the output for errors.")
        return
    end

    -- Run the server and client executables
    run_server()
    run_client()
end

vim.keymap.set('n', '<leader>rp', run_code, { noremap = true, silent = false })
vim.api.nvim_create_user_command('RunClient', run_client, { nargs = 0 })
vim.api.nvim_create_user_command('RunServer', run_server, { nargs = 0 })

