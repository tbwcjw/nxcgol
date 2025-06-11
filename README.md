<p align="center">
<img width="15%" src="assets/nx_cgol_logo.png">
</p>
    <h2 align="center">nxcgol</h2>
<p align="center">An implementation of <a href='https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life'>Conway's Game of Life</a> for the Nintendo Switch </p>
<p align="center">
    <img src="https://github.com/tbwcjw/nxcgol/actions/workflows/c.yml/badge.svg?event=push"> 
    <a href="https://tbwcjw.github.io/nxcgol/">
        <img alt="Documentation" src="https://img.shields.io/badge/documentation-orange">
    </a>
</p>
<hr>
<p align="center">
<img width="75%" src="assets/nx_cgol_animated.gif"><br>
</p>
<hr>
<h4>Setup</h4>
<ul>
    <li>Unzip <code>[release].zip</code>, copy <code>switch/</code> to the root of your SD card.</li>
    <li>Set <code>nxcgol.config.txt</code>, explained <a href='#config'>below</a>.
    <li>Launch the application from HB menu.</li>
</ul>
<table id="config">
    <thead>
        <tr>
            <th>Key</th>
            <th>Type</th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><code>simulation_speed</code></td>
            <td><code>int</code></td>
            <td><p>Between <code>0</code> and <code>100</code> (percent).</p>
            <i>Default: <code>80</code></i></td>
        </tr>
        <tr>
            <td><code>starting_density</code></td>
            <td><code>int</code></td>
            <td><p>Between <code>0</code> and <code>100</code>.
            <br>The amount of cells to spawn in a new colony.</p>
            <i>Default: <code>100</code></i></td>
        </tr>
        <tr>
            <td><code>show_stats</code></td>
            <td><code>bool</code></td>
            <td><p>Either <code>true</code> or <code>false</code>.<br>The last line of the console will display controls, the current generation and the current colony number.</p>
            <i>Default: <code>true</code></i></td>
        </tr>
        <tr>
            <td><code>auto_restart</code></td>
            <td><code>bool</code></td>
            <td><p>Either <code>true</code> or <code>false</code>.<br>Start a new colony when the current one has stagnated.</p>
            <i>Default: <code>true</code></i></td>
        </tr>
        <tr>
            <td><code>stagnant_wait</code></td>
            <td><code>int</code></td>
            <td><p>Between <code>1</code> and <code>1000</code>.<br>Wait this amount of generations to start a new colony after the current one has stagnated.</p>
            <i>Default: <code>100</code></i></td>
        </tr>
        <tr>
            <td><code>colorful</code></td>
            <td><code>bool</code></td>
            <td><p>Either <code>true</code> or <code>false</code>.<br>Use the colors, as below, to show the state of each cell, else, use black background and white cells.</p>
            <i>Default: <code>true</code></i></td>
        </tr>
        <tr>
            <td><code>stable_color</code></td>
            <td><code>str</code><br><code><i>R,G,B</i></code></td>
            <td>
                <p>Stable cells/patterns</p>
                <i>Default: <img style="vertical-align: middle;" src="https://img.shields.io/badge/86,252,3-56fc03" alt="86,252,3"></i>
            </td>
        </tr>
        <tr>
            <td><code>growth_color</code></td>
            <td><code>str</code><br><code><i>R,G,B</i></code></td>
            <td>
                <p>Patterns increase in size/complexity over time</p>
                <i>Default: <img style="vertical-align: middle;" src="https://img.shields.io/badge/3,252,219-03fcdb" alt="3,252,219"></i>
            </td>
        </tr>
        <tr>
            <td><code>dense_color</code></td>
            <td><code>str</code><br><code><i>R,G,B</i></code></td>
            <td>
                <p>Region/pattern with high concentration of live cells</p>
                Default: <img style="vertical-align: middle;" src="https://img.shields.io/badge/252,231,3-fce703" alt="252,231,3"></i>
            </td>
        </tr>
         <tr>
            <td><code>sparce_color</code></td>
            <td><code>str</code><br><code><i>R,G,B</i></code></td>
            <td>
                <p>Region with few live cells</p>
                <i>Default: <img style="vertical-align: middle;" src="https://img.shields.io/badge/252,3,3-fc0303"></i>
            </td>
        </tr>
    </tbody>
</table>
<hr>
<h4>Building</h4>
<ul>
    <li>Install <a href='https://devkitpro.org'>devkitpro</a> with <code>switch-dev</code>, <code>switch-curl</code> and <code>switch-zlib</code> using the <a href='https://devkitpro.org/wiki/Getting_Started'>Getting Started</a> guide. For building releases you will need <code>zip</code>.
    <li>Clone <a href='https://github.com/tbwcjw/nxcgol.git'>https://github.com/tbwcjw/nxcgol.git</a>.
    <li>Copy <code>nxcgol.sample.txt</code> to <code>nxcgol.config.txt</code> and fill in the fields<sup><a href='#config'>*</a></sup>.
    <li>
        <code>make (all)</code> - build the application and generate a release.
        <br>
        <code>make build</code> - build the application.
        <br>
        <code>make release</code> - builds and generate a release.
        <br>
        <code>make clean</code> - removes build/build data. does not remove releases
    </li>
    <li>Use NXLink to send <code>application.nro</code> and fptd to send the <code>nxcgol.config.txt</code>, ensuring it is in the same directory as <code>application.nro</code>.</li>
</ul>
<hr>
<h4>Licenses</h4>
This software is licensed under the MIT License.
