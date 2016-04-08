/*******************************************************************************
*
* Copyright 2015 Stefan Majewsky <majewsky@gmx.net>
*
* This file is part of Holo.
*
* Holo is free software: you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* Holo is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
* A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* Holo. If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

package impl

import (
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

//Plugin describes a plugin executable adhering to the holo-plugin-interface(7).
type Plugin struct {
	id             string
	executablePath string
}

//NewPlugin creates a new Plugin.
func NewPlugin(id string) *Plugin {
	executablePath := filepath.Join(RootDirectory(), "usr/lib/holo/holo-"+id)
	return &Plugin{id, executablePath}
}

//NewPluginWithExecutablePath creates a new Plugin whose executable resides in
//a non-standard location. (This is used exclusively for testing plugins before
//they are installed.)
func NewPluginWithExecutablePath(id string, executablePath string) *Plugin {
	return &Plugin{id, executablePath}
}

//ID returns the plugin ID.
func (p *Plugin) ID() string {
	return p.id
}

//ResourceDirectory returns the path to the directory where this plugin may
//find its resources (entity definitions etc.).
func (p *Plugin) ResourceDirectory() string {
	return filepath.Join(RootDirectory(), "usr/share/holo/"+p.id)
}

//CacheDirectory returns the path to the directory where this plugin may
//store temporary data.
func (p *Plugin) CacheDirectory() string {
	return filepath.Join(CachePath(), p.id)
}

//StateDirectory returns the path to the directory where this plugin may
//store persistent data.
func (p *Plugin) StateDirectory() string {
	return filepath.Join(RootDirectory(), "var/lib/holo/"+p.id)
}

//Command returns an os.exec.Command structure that is set up to run the plugin
//with the given arguments, producing output on the given output and error
//channels. For commands that use file descriptor 3 as an extra output channel,
//the `msg` file can be given (nil is acceptable too).
//
//Note that if a write end of an os.Pipe() is passed for `msg`, it must be
//Close()d after the child is Start()ed. Otherwise, reads from the read end
//will block forever.
func (p *Plugin) Command(arguments []string, stdout io.Writer, stderr io.Writer, msg *os.File) *exec.Cmd {
	//start the plugin process with a shell script that colorizes messages on stderr like so:
	//		!! foo bar			-> error (red)
	//		>> foo bar			-> warning (yellow)
	colorizer := `sed 's/^\(!! .*\)$/\x1B[1;31m\1\x1B[0m/;s/^\(>> .*\)$/\x1B[1;33m\1\x1B[0m/'`
	cmd := exec.Command("bash", append(
		[]string{"-c", fmt.Sprintf(`"$0" "$@" 2> >(%s)`, colorizer), p.executablePath},
		arguments...,
	)...)
	cmd.Stdin = nil
	cmd.Stdout = stdout
	cmd.Stderr = stderr
	if msg != nil {
		cmd.ExtraFiles = []*os.File{msg}
	}

	//setup environment
	env := os.Environ()
	env = append(env, "HOLO_API_VERSION=3")
	env = append(env, "HOLO_CACHE_DIR="+normalizePath(p.CacheDirectory()))
	env = append(env, "HOLO_RESOURCE_DIR="+normalizePath(p.ResourceDirectory()))
	env = append(env, "HOLO_STATE_DIR="+normalizePath(p.StateDirectory()))
	if os.Getenv("HOLO_ROOT_DIR") == "" {
		env = append(env, "HOLO_ROOT_DIR="+normalizePath(RootDirectory()))
	}
	cmd.Env = env

	return cmd
}

//RunCommandWithFD3 extends the Command function with automatic setup and
//reading of the file-descriptor 3, that is used by some plugin commands to
//report structured messages to Holo.
func (p *Plugin) RunCommandWithFD3(arguments []string, stdout, stderr io.Writer) (string, error) {
	//the command channel (file descriptor 3 on the side of the plugin) can
	//only be set up with an *os.File instance, so use a pipe that the plugin
	//writes into and that we read from
	cmdReader, cmdWriterForPlugin, err := os.Pipe()
	if err != nil {
		return "", err
	}

	//execute apply operation
	cmd := p.Command(arguments, stdout, stderr, cmdWriterForPlugin)
	err = cmd.Start() //cannot use Run() since we need to read from the pipe before the plugin exits
	if err != nil {
		return "", err
	}

	cmdWriterForPlugin.Close() //or next line will block (see Plugin.Command docs)
	cmdBytes, err := ioutil.ReadAll(cmdReader)
	if err != nil {
		return "", err
	}
	err = cmdReader.Close()
	if err != nil {
		return "", err
	}
	return string(cmdBytes), cmd.Wait()
}

//For reproducibility in tests.
func normalizePath(path string) string {
	if path == "/" {
		return "/"
	}
	//remove leading "./" from relative paths
	path = strings.TrimPrefix(path, "./")
	//remove trailing slash
	return strings.TrimSuffix(path, "/")
}
